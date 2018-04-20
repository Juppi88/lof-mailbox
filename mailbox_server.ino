////////////////////////////////////////
// Turku LoF LoRa Mailbox IoT project
//
// Mailbox state server
// --------------------
// Server receives mailbox state from client and sends state over serial to e.g. 
// Raspberry pi that can forward the information to IoT-Ticket.
//
// Code is based on reliable messaging server with the RHReliableDatagram class, 
// using the RH_RF95 driver to control a RF95 radio. Server is designed to work 
// with client that is based on example RF95 Reliable datagram client.
////////////////////////////////////////

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

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

// Singleton instance of the radio driver for Adafruit Feather M0 with RFM95 
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(rf95, SERVER_ADDRESS);

// Sensor status struct, to be sent to the central unit (RPI).
struct status_t {
  bool has_mail;
  uint16_t ir_distance; // [mm]
  uint16_t sonar_distance; // [mm]
};

uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

////////////////////////////////////////

static void blink(int times);
static void send_sensor_data_to_serial(status_t &status);

////////////////////////////////////////

void setup() 
{
  unsigned long start = millis();
    
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  
  // Wait for serial port to be available
  while (!Serial) {
    if (millis() - start > WAIT_FOR_SERIAL_MS) {
      break;
    }
  }

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
  // Lets change frequency to European LoRa, 868.0MHz
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  blink(5);
}

////////////////////////////////////////

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("----------------------------------------");
      Serial.print("Got request from client: 0x");
      Serial.println(from, HEX);

      // Unpack the sruct
      status_t status;
      memcpy(&status, buf, sizeof(status));
    
      Serial.print("IR: ");
      Serial.println(status.ir_distance);

      if (status.has_mail) {
        Serial.println("!!! Mailbox has mail !!!");
      }
      else {
        Serial.println("There is no mail in the mailbox.");
      }
      
      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from)) {
        Serial.println("sendtoWait failed");
      }
      
      Serial.println("Acknowledged!");
      digitalWrite(LED_BUILTIN, LOW);

      // Send status over serial
      send_sensor_data_to_serial(status);
      
      Serial.print("\n");
    }
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

static void send_sensor_data_to_serial(status_t &status)
{
  Serial.println("-- TO Raspberry -->"); 
  if (status.has_mail) {
    Serial.println("M:1");
  }
  else {
    Serial.println("M:0");
  }
  Serial.print("IR:");
  Serial.println(status.ir_distance);
  Serial.println("<-- TO Raspberry --"); 
}


