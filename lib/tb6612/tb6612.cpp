#include "tb6612.h"

//which pins connect to the tb6612
const byte STBY = 13;	//Standby/Coast
const byte PWMA = 12;	//Speed control. Must be an PWM enabled pin 
const byte AIN1 = 10;	//control 1
const byte AIN2 = 14;	//Control 2

TB6612::TB6612(){
    pinMode(STBY, OUTPUT);
    pinMode(PWMA, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    analogWrite(PWMA, 0);	//set power to zero
}

TB6612::~TB6612(){/*nothing to destruct*/}

//power 0-255
void TB6612::move(int power, bool direction){
	coast(0);
	if(direction){
		digitalWrite(AIN1, LOW);
		digitalWrite(AIN2, HIGH);
	}
	else{
		digitalWrite(AIN1, HIGH);
		digitalWrite(AIN2, LOW);
	}
	
	analogWrite(PWMA, power);
}

void TB6612::brake(bool brake){
	//see datasheet;; both inputs high activate brake. Only if not coasting 
	digitalWrite(AIN1, HIGH);
	digitalWrite(AIN2, HIGH);
}

void TB6612::coast(bool coast){
	//STBY pin set "high impedance" mode
	if(coast)
		digitalWrite(STBY, LOW);
	else
		digitalWrite(STBY, HIGH);
		
}
