/** GUIDE BLADE
This library uses direct access to the AVR pins, not using Arduino code
This might interfer with a=Arduino specific stuff.
It uses the pin change interrupt (see datahseet).
Moteino's analog pins A0 and A1 are used. These are part of portA in AVR terms
**/
int16_t position = 0;
int skip = 0;
uint8_t pina, previousX4;

void setup()
{
	Serial.begin(115200);
	DDRA &= 0b11111100;		//all inputs
	PORTA &= 0b11111100;		//turn off pullups
	cli();  
	PCICR |= 0x01;			// Enable PCINT0 (port A) interrupt
	PCMSK0 |= 0b00000011;	//interrupt on pins A0 and A1
	sei();
	previousX4 = PINA&0x03;
}

void loop() {
	Serial.print(position,DEC);
	Serial.print("\t");
	Serial.println(skip,DEC);
	delay(100);
}

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
			skip++;
	}
	previousX4 = pina;
}
