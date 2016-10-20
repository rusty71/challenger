#include <Arduino.h>
#line 1 "/home/jeroen/dev/moteino/challenger/adxl345/src/adxl345.ino"
#line 1 "/home/jeroen/dev/moteino/challenger/adxl345/src/adxl345.ino"
#include <adxl345.h>

ADXL345 accel;

#line 5 "/home/jeroen/dev/moteino/challenger/adxl345/src/adxl345.ino"
void setup();
#line 18 "/home/jeroen/dev/moteino/challenger/adxl345/src/adxl345.ino"
void loop();
#line 5 "/home/jeroen/dev/moteino/challenger/adxl345/src/adxl345.ino"
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
	Serial.print(xyz[0]/32.0);
	Serial.print("\t");
	Serial.print(xyz[1]/32.0);
	Serial.print("\t");
	Serial.println(xyz[2]/32.0);
}



