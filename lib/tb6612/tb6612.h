/*
  tb6612.h - Library for running a motor on the A part of a tb6612 driver.
  The B motor is not implemented (yet)
*/

#ifndef TB6612_H
#define TB6612_H

#include "Arduino.h"

class TB6612 { 
public:
	TB6612();
	~TB6612();
    void move(int power, bool direction);
    void brake(bool brake);
	void coast(bool coast);
};

#endif
