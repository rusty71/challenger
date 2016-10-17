#include <blade.h>

BLADE steer;

void setup()
{
	Serial.begin(115200);

	steer.begin();
	Serial.println("Calibrate in one seconde");
	delay(1000);
	steer.reset();
	Serial.println("start");
}

int16_t pos,prev_pos;
void loop() {
	pos = steer.get();
	if(pos != prev_pos) {
		Serial.println(pos);
		prev_pos = pos;
	}
	
}
