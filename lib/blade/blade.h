/** GUIDE BLADE
This library uses direct access to the AVR pins, not using Arduino code
This might interfer with a=Arduino specific stuff.
It uses the pin change interrupt (see datahseet).
Moteino's analog pins A0 and A1 are used. These are part of portA in AVR terms
**/

#ifndef BLADE_H
#define BLADE_H

#include "Arduino.h"

class BLADE { 
public:
	BLADE();
	~BLADE();
    void begin();
    void reset(void);			//resets count to zero
    void set(int16_t position);	//sets current reference
    int16_t get(void);
};

#endif
