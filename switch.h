#ifndef __SWITCH_H
#define __SWITCH_H

class Switch
{
 private:
  int pin;
  int state;

public:
  Switch(int switch_pin);
  
  bool is_falling(void);
  bool is_rising(void);
};

#endif

