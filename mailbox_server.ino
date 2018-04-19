// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

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

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Adafruit Feather M0 with RFM95 

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(rf95, SERVER_ADDRESS);

// Sensor status struct, to be sent to the central unit (RPI).
struct status_t {
  bool has_mail;
  uint16_t ir_distance; // [mm]
  uint16_t sonar_distance; // [mm]
};

////////////////////////////////////////

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
  
  if (!manager.init())
    Serial.println("init failed");

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
}

uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

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

