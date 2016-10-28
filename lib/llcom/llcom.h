/*
 * This 'library' tries to offer low-latency communication for a 1-n network
 * The master continiously pings the slaves. The slaves must ping back with a timeout
 * The 'pings' can contain datagrams.
 * TODO:
 * -multiple slaves
 * -variable, queue based, timeouts
 * 
 */

#ifndef LLCOM_H
#define LLCOM_H
#include "Arduino.h"
#endif
#include "RHReliableDatagram.h"

#define BASE_ADDRESS (0)
#define CAR_ADDRESS (1)


#define MAX_MSG_SIZE (64)
#define MSG_BUF_SIZE (16)			//power of 2
#define BUF_SIZE_MASK (0x0f)		//power of 2
typedef enum state_t {
						INIT,
						POLL,
						WAITING,
						REPEAT,
						REPLY,
						SENDING
					 };


typedef enum message_t {
						SEQ,
						ASCII
					 };

extern "C"
{
	// callback functions always follow the signature: void cmd(void);
	typedef void(*commsCallback) (uint8_t *, uint8_t);
}

class LLSLAVE { 
public:
	LLSLAVE(RHDatagram* driver);
	~LLSLAVE();
	bool send(uint8_t *msg, uint8_t len);
	bool send(message_t msg_type, char *msg);
	void schedule(void);
	uint8_t head = 0, tail = 0;
	uint8_t queue_len(void);
	void attach(commsCallback func);


private:
	state_t rfstate = INIT;
	RHDatagram *dgram;
	commsCallback callback;            // default callback function  

	
	uint8_t len, to, from, id, flags;

	//message queue ring buffer
	uint8_t mesg_buf[MSG_BUF_SIZE][MAX_MSG_SIZE+1];	//first byte is length
	//receive buffer and  parameters
	uint8_t send_buf[MAX_MSG_SIZE];
	uint8_t recv_buf[MAX_MSG_SIZE];

	uint8_t last_id;			//last received id
	uint16_t duplicate = 0;		//duplicate id's received
	uint16_t missed = 0;		//missed packets. diffenence from duplicate?
	uint16_t would_block = 0;	//would block on sendto()

	
	bool push_msg(uint8_t *msg, uint8_t len);
	uint8_t pop_msg(uint8_t *msg);

	void printmsg(uint8_t *msg);
};

class LLMASTER { 
public:
	LLMASTER(RHDatagram* driver);
	~LLMASTER();
	bool send(uint8_t *msg, uint8_t len);
	void schedule(void);
	//~ uint8_t head = 0, tail = 0;
	//~ uint8_t queue_len(void);
	void attach(commsCallback func);
	uint8_t seq;
	uint32_t reply_timer = 0;
	uint16_t timeout = 0;
	uint16_t reply_timeout = 50;	//ms
	uint16_t msg_count = 0;

private:
	state_t rfstate = INIT;
	RHDatagram *dgram;
	commsCallback callback;            // default callback function  


	uint8_t len, to, from, id, flags;

	//receive buffer and  parameters
	uint8_t send_buf[MAX_MSG_SIZE];
	uint8_t recv_buf[MAX_MSG_SIZE];

	uint8_t last_id;			//last received id
	uint16_t duplicate = 0;		//duplicate id's received
	uint16_t missed = 0;		//missed packets. diffenence from duplicate?
	uint16_t would_block = 0;	//would block on sendto()

	
	void printmsg(uint8_t *msg);
};



