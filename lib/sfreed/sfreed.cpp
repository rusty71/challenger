#include "sfreed.h"

const byte SF_PIN = 11;

#define DEBOUNCE_MS 100


//only one callback possible
sfCallback callback = NULL;

//interrupt shared variables need to be volatile
volatile uint16_t sfcount = 0;
volatile long long debounce = 0;

void SF_INT(void) {
	if((millis() - debounce)>DEBOUNCE_MS) {
		sfcount++;
		if (callback != NULL) (*callback)();
		debounce = millis();
	}
	//else debounce
}

SFREED::SFREED(){}

SFREED::~SFREED(){}

void SFREED::begin(void){
	pinMode(SF_PIN, INPUT_PULLUP);
	attachInterrupt(1, SF_INT, FALLING);
}

void SFREED::reset(){
	sfcount = 0;
}

void SFREED::attach(sfCallback func){
	callback = func;
}

uint16_t SFREED::get(){
	return sfcount;
}
