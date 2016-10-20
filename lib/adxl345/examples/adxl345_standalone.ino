#include <Wire.h>

#define DEVICE (0xA7 >> 1)

byte _buff[6];

char POWER_CTL = 0x2D;
char DATA_FORMAT = 0x31;
char DATAX0 = 0x32;

void setup() {
	Wire.begin();
	Serial.begin(115200);
	Serial.print("init");
	//0x03 16G range
	writeTo(DATA_FORMAT, 0x03);
	//start sensor
	writeTo(POWER_CTL, 0x08);
}

void loop() {
  readAccel();
  delay(50);
}

void readAccel() {
	readFrom( DATAX0, 6, _buff);
	//need to calibrate
	int16_t x = ((((int16_t)_buff[1]) << 8) | _buff[0])-4;
	int16_t y = ((((int16_t)_buff[3]) << 8) | _buff[2])+3;
	int16_t z = ((((int16_t)_buff[5]) << 8) | _buff[4])+4;
}

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

