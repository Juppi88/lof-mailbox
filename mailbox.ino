#include <Wire.h>
#include <VL6180X.h>
#include <NewPing.h>

const int SWITCH_PIN = 7;

static void blink(int times);
static void on_box_opened(void);

VL6180X sensor;
NewPing sonar(5, 4, 200);

////////////////////////////////////////

void setup()
{
  // Initialize the distance sensor.
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
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), on_box_opened, FALLING);
}

////////////////////////////////////////

void loop()
{
  uint16_t distance;

  // Read distance from the ultrasonic sensor.
  distance = distance = sonar.convert_cm(sonar.ping_median(5));

   // Read distance from the other sensor.
  distance = sensor.readRangeSingleMillimeters();
  
  //Serial.println(distance);
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

static void on_box_opened(void)
{
  Serial.println("Interruptaannun täällä nyt");
}


