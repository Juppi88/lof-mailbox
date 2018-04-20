# lof-mailbox
*Snail mail detector. Turku office entry for the Wapice Leap of Fate 2018 event.*

Wapice Turku office is located on the 4th floor of EuroCity close to entrance B. The mailbox for the office recides on the first floor at entrance C. Map of the building: [Turku Science Park Map](https://www.teknologiakiinteistot.fi/files/Turku%20Science%20Park%20kartta%287%29.pdf)

It takes 3 minutes 11 seconds to walk to the mailbox and back, so it would save a lot of resources if one could see the state of the mailbox from e.g. an IoT dashboard. To accomplish this a "snail mail detector" is needed. Snail mail detector provides information about the state of the mailbox. This information is then transferred over LoRaWAN from the mailbox to the office and from there to [IoT-Ticket](https://iot-ticket.com/).

This repository contains the result of two separate projects, one for detecting mail in a mailbox and one for transferring that data over LoRaWAN to an IoT dashboard.

### HW, SW, Tools etc used:
* Power / Battery (post box side + office side)
* 2x LoRaWAN transceiver (post box side + office side)
* Raspberry Pi (office side) 
* Arduino Micro (post box side, used for developing and testing the sensors)
* IoT-Ticket (for Turku office dashboard, could contain other stuff as well in the future)
* Pololu VL6180x proximity sensor (for detecting mail in the box)
* 2x microswitches (for detecting when the mailbox is opened)
* HC-SR05 ultrasonic proximity sensor (unused)

### Links:
* [Adafruit Feather M0 Radio with LoRa Radio Module](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/overview)
* [LoraWAN-in-C library, adapted to run under the Arduino environment](https://github.com/matthijskooijman/arduino-lmic)
* [RadioHead Packet Radio library for embedded microprocessors](http://www.airspayce.com/mikem/arduino/RadioHead/)
* [Radio Range Test with RFM69HCW (contains code that use RadioHead Packet Radio library)](http://www.rocketscream.com/blog/2016/03/10/radio-range-test-with-rfm69hcw/)
