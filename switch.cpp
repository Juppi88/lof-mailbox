#include <Arduino.h>
#include "switch.h"

Switch::Switch(int switch_pin)
{
  pin = switch_pin;
  state = HIGH;

  // Enable the pullup resistor.
  pinMode(switch_pin, INPUT_PULLUP);
}

bool Switch::is_falling(void)
{
  int current_state = digitalRead(pin);

  // Pin state changed from high to low.
  if (state == HIGH && current_state == LOW) {

    delay(5);
    current_state = digitalRead(pin);
    
    // If the state is still low after 5ms, declare the state changed.
    if (current_state == LOW) {
      
      state = current_state;
      return true;
    }
  }

  state = current_state;
  return false;
}

bool Switch::is_rising(void)
{
  int current_state = digitalRead(pin);

  if (state == LOW && current_state == HIGH) {

    delay(5);
    current_state = digitalRead(pin);
    
    if (current_state == HIGH) {
      
      state = current_state;
      return true;
    }
  }

  state = current_state;
  return false;
}

