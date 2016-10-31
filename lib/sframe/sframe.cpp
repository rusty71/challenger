#include "sframe.h"

SFRAME::SFRAME(){
	recv_ptr = 0;
}

SFRAME::~SFRAME(){}

void SFRAME::attach(recvCallback func){
	callback = func;
}

//pack and send to serial host.
void SFRAME::sendframe(uint8_t *msg, uint8_t len)
{
	uint8_t i;

	Serial.write(HDR);
	for(i = 0; i < len; i++){
		if((msg[i]==ESC)||(msg[i]==HDR)||(msg[i]==FOOT)) {
			Serial.write(ESC);	//escape that shit
		}
		Serial.write(msg[i]);
	}
	Serial.write(FOOT);
}


//schedules reading of serial data
void SFRAME::schedule() {
	int rchar;

	while (Serial.available()) {
		rchar = Serial.read();
		if(rchar == -1)
			break;
		switch(recv_state)
		{
			case WAIT_FOR_HEADER:
				switch((uint8_t)rchar){
					case HDR:
						recv_state=IN_MSG;
						recv_ptr=0;
						break;
					case ESC:
					case FOOT:
					   break;
				}
				break;
			case IN_MSG:
				switch((uint8_t)rchar){
					case ESC:
						recv_state=IN_ESC;
						break;
					case FOOT:
						recv_state=WAIT_FOR_HEADER;
						if (callback != NULL) (*callback)(recv_buf, recv_ptr);
						recv_ptr = 0;
						break;
					default:
						recv_buf[recv_ptr++]=(uint8_t)rchar;
				}
				break;
			case IN_ESC:
				recv_state=IN_MSG;
				recv_buf[recv_ptr++]=(uint8_t)rchar;
				break;
		}
	}
}
