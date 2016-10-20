#include "blade.h"

const byte T1_PIN = 1;
volatile int16_t position = 0;
int skips = 0;
uint8_t pina, previousX4;

BLADE::BLADE(){/*TODO: not called when expected*/}

BLADE::~BLADE(){/*stop counting here*/}

void BLADE::begin(){
	DDRA &= 0b11111100;		//all inputs
	PORTA &= 0b11111100;	//turn off pullups
	cli();  
	PCICR |= 0x01;			// Enable PCINT0 (port A) interrupt
	PCMSK0 |= 0b00000011;	//interrupt on pins A0 and A1
	sei();
	previousX4 = PINA&0x03;
}

void BLADE::reset(){
	position = 0;
}

void BLADE::set(int16_t position){
	position = position;
}

int16_t BLADE::get(){
	return position;
}

/*This interrupt is called on every state change of pins A0 and A1*/
ISR(PCINT0_vect) {
	//safe current X4 value
	pina = PINA&0x03;
	//switch based on current and previous X4 in LSNibble see: https://en.wikipedia.org/wiki/Rotary_encoder
	switch((previousX4<<2)|pina) {
	   case 0b00000001:
			position++;	
		  break;		
	   case 0b00000010:
			position--;
			break;		
	   case 0b00000100:
			position--;
			break;		
	   case 0b00000111:
			position++;
			break;		
	   case 0b00001000:
			position++;
			break;		
	   case 0b00001011:
			position--;
			break;		
	   case 0b00001101:
			position--;
			break;		
	   case 0b00001110:
			position++;
			break;		
		default: //all equal or skipped steps. could be debounce or real mis.
			skips++;
	}
	previousX4 = pina;
}
