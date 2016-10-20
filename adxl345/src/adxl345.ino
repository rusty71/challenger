#include <adxl345.h>

ADXL345 accel;

void setup()
{
	//need to explicity call begin() because constructor is not called?!? FIXME
	accel.begin();
	Serial.begin(115200);
	delay(100);
	Serial.println("Starting...");
	delay(100);
}


int16_t xyz[3];

void loop()
{

	accel.get(xyz);
	//divide by 32.0 for 16G scale
	Serial.print(xyz[0]/32.0);
	Serial.print("\t");
	Serial.print(xyz[1]/32.0);
	Serial.print("\t");
	Serial.println(xyz[2]/32.0);
}


