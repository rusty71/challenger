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
	writeTo(DATA_FORMAT, 0x01);
	//start sensor
	writeTo(POWER_CTL, 0x08);
}

//~ void ADXL345::get(int16_t *xyz){
	//~ int16_t x_min=0, x_max=0;
	//~ int16_t y_min=0, y_max=0;
	//~ int16_t z_min=0, z_max=0;
	//~ int16_t tmp;
	//~ int16_t x_sum = 0, y_sum=0, z_sum=0;

	//~ for(byte i=0; i < 18; i++) {
		//~ //read x,y,z values from sensor in tmp array
		//~ readFrom( DATAX0, 6, _buff);

		//~ tmp = ((((int16_t)_buff[1]) << 8) | _buff[0]);
		//~ x_min = min(x_min, tmp);
		//~ x_max = max(x_max, tmp);
		//~ x_sum += tmp;

		//~ tmp = ((((int16_t)_buff[3]) << 8) | _buff[2]);
		//~ y_min = min(y_min, tmp);
		//~ y_max = max(y_max, tmp);
		//~ y_sum += tmp;

		//~ tmp = ((((int16_t)_buff[5]) << 8) | _buff[4]);
		//~ z_min = min(z_min, tmp);
		//~ z_max = max(z_max, tmp);
		//~ z_sum += tmp;
	//~ }
	//~ //remove minimum and maximum
	//~ x_sum = x_sum - x_min - x_max;
	//~ y_sum = y_sum - y_min - y_max;
	//~ z_sum = z_sum - z_min - z_max;
	
	//~ xyz[0] = (x_sum >> 4);
	//~ xyz[1] = (y_sum >> 4);
	//~ xyz[2] = (z_sum >> 4);
	
//~ }

//returns x,y,z. Needs to be divided by 32.0, for 16G range, to get float values
void ADXL345::get(int16_t *xyz){
	readFrom( DATAX0, 6, _buff);
	//need to calibrate
	xyz[0] = ((((int16_t)_buff[1]) << 8) | _buff[0])-4;		//TODO fix static calibration
	xyz[1] = ((((int16_t)_buff[3]) << 8) | _buff[2])+3;
	xyz[2] = ((((int16_t)_buff[5]) << 8) | _buff[4])+4;
}
