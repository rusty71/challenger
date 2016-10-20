#include <tachometer.h>

TACHOMETER wheel_rev;
int LED = 15;

uint16_t prev;

void setup()
{
	//need to explicity call begin() because constructor is not called?!? FIXME
	wheel_rev.begin(36);
	Serial.begin(115200);
	delay(100);
	Serial.println("Starting...");
	delay(100);
	prev = wheel_rev.get();

}


void loop()
{
	uint16_t val;
	if( (val = wheel_rev.get()) != prev) {
		prev = val;
		Serial.println(val);
		if(val > 25)
			wheel_rev.reset();
	}
}

