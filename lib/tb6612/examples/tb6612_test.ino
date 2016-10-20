#include <tb6612.h>

TB6612 Motor;

int power = 0;

void setup(){
	Serial.begin(115200);
	Motor.coast(0);
	Motor.move(255,0);
	delay(2000);
	Serial.println("break:1");
	Motor.brake(1);
	delay(1500);
	Motor.brake(0);
	Motor.move(255,0);
	delay(2000);
	Serial.println("coast:1");
	Motor.coast(1);
	delay(1500);
	Motor.coast(0);
}


void loop(){
	for(power = 0; power < 255;power + 10) {
		Motor.move(power,1);	
		delay(50);
	}
}
