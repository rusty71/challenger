#include "adxl345.h"
#include <Wire.h>

//i2c address
#define DEVICE (0xA7 >> 1)

byte _buff[6];

//ADXL345 registers, see datasheet
char POWER_CTL = 0x2D;
char DATA_FORMAT = 0x31;
char DATAX0 = 0x32;

void writeTo(byte address, byte val) {
	Wire.beginTransmission(DEVICE);
	Wire.write(address);
	Wire.write(val);
	Wire.endTransmission();
}
  
void readFrom(byte address, int num, byte _buff[]) {
	Wire.beginTransmission(DEVICE);
		Wire.write(address);
	Wire.endTransmission();
	Wire.requestFrom(DEVICE, num);
	int i = 0;
	while(Wire.available()) {
		_buff[i] = Wire.read();
		i++;
	}
	Wire.endTransmission();
}

ADXL345::ADXL345(){
}

ADXL345::~ADXL345(){/*nothing to destruct*/}


//sets range 16G
void ADXL345::begin() {
	Wire.begin();
	//0x03 16G range
	writeTo(DATA_FORMAT, 0x03);
	//start sensor
	writeTo(POWER_CTL, 0x08);
}


//returns x,y,z. Needs to be divided by 32.0, for 16G range, to get float values
void ADXL345::get(int16_t *xyz){
	readFrom( DATAX0, 6, _buff);
	//need to calibrate
	xyz[0] = ((((int16_t)_buff[1]) << 8) | _buff[0])-4;		//TODO fix static calibration
	xyz[1] = ((((int16_t)_buff[3]) << 8) | _buff[2])+3;
	xyz[2] = ((((int16_t)_buff[5]) << 8) | _buff[4])+4;
}
