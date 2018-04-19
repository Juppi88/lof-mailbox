//#define WITH_SONAR

#include <Wire.h>
#include <VL6180X.h>
#include "switch.h"

#ifdef WITH_SONAR
#include <NewPing.h>
#endif

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
Switch door_switch(8);
Switch hatch_switch(9);

static volatile long timer;

////////////////////////////////////////

static void blink(int times);
//static void on_box_opened(void);
static status_t check_sensors(void);
static void send_sensor_data(status_t &status);

////////////////////////////////////////

void setup()
{
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
  while (!Serial) { }

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
  
  Serial.print("\n");
}

