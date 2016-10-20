/*
 * sfreed.h
 * A library for detecting thge Start Finish line
 * A magnet under the track triggers a reed switch in the car
 * It is handled by External Interupt 1 EXT_INT1, pin 11 on the MoteinoMEGA
 * The callback is driver from the ISR, do not use delay() and friends
*/

#ifndef SFREED_H
#define SFREED_H

#include "Arduino.h"

extern "C"
{
	// callback typedef
	typedef void(*sfCallback) (void);
}

class SFREED { 
public:
	SFREED();
	~SFREED();
    void begin(void);
    void reset(void);
    uint16_t get(void);
    void attach(sfCallback);
};

#endif
