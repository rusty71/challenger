#ifndef SFRAME_H
#define SFRAME_H

#include "Arduino.h"

extern "C"
{
	// callback typedef
	typedef void(*recvCallback) (uint8_t *msg, uint8_t len);
}

//special characters for framing
#define ESC 0x7c
#define HDR 0x7d
#define FOOT 0x7e

//message receiver state machine
enum STATES {
    WAIT_FOR_HEADER,
    IN_MSG,
    IN_ESC
};

class SFRAME { 
public:
	SFRAME();
	~SFRAME();
	void sendframe(uint8_t *msg, uint8_t len);
	void attach(recvCallback);
	void schedule();

private:
	uint8_t recv_buf[64];
	volatile uint8_t recv_ptr; 
	volatile uint8_t recv_state=WAIT_FOR_HEADER;
	//only one callback possible
	recvCallback callback = NULL;
};

#endif
