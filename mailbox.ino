#include <Wire.h>
#include <VL6180X.h>
#include <NewPing.h>

VL6180X sensor;
NewPing sonar(5, 4, 200);

void setup()
{
  // Initialize the distance sensor.
  Wire.begin();
  
  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);

  // Enable built-in LED for debugging.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(57600);
  while (!Serial) { }
}

void loop()
{
  uint16_t distance = sensor.readRangeSingleMillimeters();

  digitalWrite(LED_BUILTIN, distance < 100 ? HIGH : LOW);

  int distance2 = sonar.convert_cm(sonar.ping_median(5));
  Serial.println(distance2);
  
  delay(100);
}

