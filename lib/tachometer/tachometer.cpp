#include "tachometer.h"

const byte T1_PIN = 1;
uint16_t revolutions;	//internal counter

//FIXME: why is the constructor not callled ?!?
TACHOMETER::TACHOMETER(){/*TODO: not called when expected*/}

TACHOMETER::~TACHOMETER(){/*stop counting here*/}

//TODO: check why this cant be done in the constructor
void TACHOMETER::begin(uint16_t ppr){
    pinMode(T1_PIN, INPUT);

	TCNT1=0x00;          //Reset Timer 1 Counter.
	TCCR1A=0x00;
	TCCR1B=0x07;       // To set all pins CSn2,CSn1,CSn0 to use external clock source on Tn Pin. Clock on rising edge.
	//Enable input capture and overflow interrupts
	TCCR1A |= 0;   
	// not required since WGM11:0, both are zero (0)
	TCCR1B |= (1 << WGM12)|(1 << CS11)|(1 << CS10);   
	// Mode = CTC, Prescaler = 64
	TIMSK1|=(1<<OCF1A);
	revolutions = 0;
	OCR1A = ppr;		//see datasheet chap.14
}

void TACHOMETER::reset(){
	revolutions = 0;
}

void TACHOMETER::set(uint16_t ppr){
	OCR1A = ppr;		//see datasheet chap.14
}

uint16_t TACHOMETER::get(){
	return revolutions;
}

//this is the ISR called when counter register (TCNT1) reaches the compare register (OCR1A)
//TCNT1 is reset to zero automatically
//this is a interrupt service routing. Dont delay(), Serial() or anything long/blokking.
ISR (TIMER1_COMPA_vect)
{
	revolutions++;
}
