/*
	adxl345.h basic accelerometer library
	* only does 16G reange, returns integers
*/

#ifndef ADXL345_H
#define ADXL345_H

#include "Arduino.h"

class ADXL345 { 
public:
	ADXL345();
	~ADXL345();
	void begin(void);
	void get(int16_t * xyz);
};

#endif
