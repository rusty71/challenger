/*
  tachometer.h - Library for counting revolutions and measuring distance.

	This library uses timer1 hardware (chapter 14 Datasheet). timer1 is also used by the servo library so cannot be used in conbination
	timer1 is setup as counter with "external clock source" on pin T1 (arduino name PB1).
	The front wheel pulse counter does 36 (35?) counts per axle rotation and is connected to pin PB1 (moteino name 1).
	The "compare match unit" of timer is setup to generate an interrupt every X pulses which can be set by tachometer::set(pulses per revolution)
	If set 36 the internal counter is incremented exactly every wheel rotation
	The main loop should poll the internal counter by tachometer::get(); to detect number of revolutions
	
*/

#ifndef TACHOMETER_H
#define TACHOMETER_H

#include "Arduino.h"

class TACHOMETER { 
public:
	TACHOMETER();
	~TACHOMETER();
    void begin(uint16_t ppr);
    void reset(void);	//resets count to zero
    void set(uint16_t ppr);
    uint16_t get(void);		//returns revolutions
};

#endif
