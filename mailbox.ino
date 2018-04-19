//#define WITH_SONAR

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include <VL6180X.h>
#include "switch.h"

#ifdef WITH_SONAR
#include <NewPing.h>
#endif

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// For feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Frequency, must match RX's freq!
#define RF95_FREQ 868.0

// Milliseconds to wait for serial
#define WAIT_FOR_SERIAL_MS 5000

const int SWITCH_PIN = 7; // Pin for the door switch. Triggers an interrupt.
const int SENSOR_CHECK_DELAY = 10000; // Time after which to check sensors after the box has been opened [ms]
const int PROXIMITY_LIMIT = 90; // Distance which counts as proximity [mm]

// Sensor status struct, to be sent to the central unit (RPI).
struct status_t {
  bool has_mail;
  uint16_t ir_distance; // [mm]
  uint16_t sonar_distance; // [mm]
};

VL6180X sensor;

#ifdef WITH_SONAR
NewPing sonar(5, 4, 200);
#endif

// Switches for the mailbox doors.
Switch door_switch(9);
Switch hatch_switch(10);

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Adafruit Feather M0 with RFM95 

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(rf95, CLIENT_ADDRESS);

static volatile long timer;

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

////////////////////////////////////////

static void blink(int times);
//static void on_box_opened(void);
static status_t check_sensors(void);
static void send_sensor_data(status_t &status);

////////////////////////////////////////

void setup()
{
  // Take start time
  unsigned long start = millis();
    
  // Initialize the IR distance sensor.
  Wire.begin();
  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);

  // Enable built-in LED for debugging.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Start serial interface.
  Serial.begin(57600);
  
  // Wait for serial port to be available
  while (!Serial) {
    if (millis() - start > WAIT_FOR_SERIAL_MS) {
      break;
    }
  }

  // Initialize Reliable datagram manager
  if (!manager.init()) {
    Serial.println("init failed");
  }
    // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
//  rf95.setTxPower(23, false);
  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);

  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Attach an interrupt to the switch pin.
  //pinMode(SWITCH_PIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), on_box_opened, FALLING);
}

////////////////////////////////////////

void loop()
{
  if (timer != 0 && millis() >= timer) {

    // Read the status of sensors and send them to the central unit.
    status_t sensors = check_sensors();
    send_sensor_data(sensors);

    // Reset the timer.
    timer = 0;
  }

  // Mailbox hatch is opened, read the status of the sensors after a while.
  if (hatch_switch.is_falling()) {
    Serial.println("Mailbox hatch was opened!");
    start_sensor_read();
  }

  // Mailbox door is closed, ditto.
  if (door_switch.is_rising()) {
    Serial.println("Mailbox door was closed!");
    start_sensor_read();
  }
}

////////////////////////////////////////

static void blink(int times)
{
  for (int i = 0; i < times; i++) {
    
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

/*static void on_box_opened(void)
{
  if (timer == 0) {
    Serial.println("Mail box was opened!");
    
    // Start a timer and check the distance sensors when it times out (this happens in loop()).
    timer = millis() + SENSOR_CHECK_DELAY;
  }
}*/

static void start_sensor_read(void)
{
  if (timer == 0) {
    // Start a timer and check the distance sensors when it times out (this happens in loop()).
    timer = millis() + SENSOR_CHECK_DELAY;
  }
}

static status_t check_sensors(void)
{
  status_t status;

  // Read distance from the other sensor.
  status.ir_distance = sensor.readRangeSingleMillimeters();

#ifdef WITH_SONAR
  // Read distance from the ultrasonic sensor [cm].
  status.sonar_distance = sonar.convert_cm(sonar.ping_median(5)) * 10;

  // If any of the sensors detects proximity, there is probably mail in the box.
  status.has_mail = (status.sonar_distance <= PROXIMITY_LIMIT || status.ir_distance <= PROXIMITY_LIMIT);
#else
  status.has_mail = (status.ir_distance <= PROXIMITY_LIMIT);
#endif

  return status;
}

static void send_sensor_data(status_t &status)
{
  Serial.println("----------------------------------------");
  Serial.println("Status of mailbox sensors:");
  Serial.println("Sending to RF95 Server");
  
  Serial.print("IR: ");
  Serial.println(status.ir_distance);

#ifdef WITH_SONAR
  Serial.print("Sonar: ");
  Serial.println(status.sonar_distance);
#endif

  if (status.has_mail) {
    Serial.println("!!! Mailbox has mail !!!");
  }
  else {
    Serial.println("There is no mail in the mailbox.");
  }

  uint8_t data[sizeof(struct status_t)];
  memcpy(data, &status, sizeof(struct status_t));

  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply! Is RF95 Server running?");
    }
  }
  else {
    Serial.println("sendtoWait failed");
  }
  
  Serial.print("\n");
}

