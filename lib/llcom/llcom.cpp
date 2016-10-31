#include "llcom.h"
#include "RHReliableDatagram.h"


bool debug=0;


LLSLAVE::LLSLAVE(RHDatagram* driver){
	dgram = driver;
	rfstate = INIT;
	callback = NULL;
}

LLSLAVE::~LLSLAVE(){};

bool LLSLAVE::send(uint8_t *msg, uint8_t len)
{
	push_msg(msg, len);
}

bool LLSLAVE::send(message_t msg_type, char *msg)
{
	char tmp[MAX_MSG_SIZE];
	uint8_t len = strlen(msg) + 1;	//add one for message type

	tmp[0] = msg_type;
	strcpy(&tmp[1], msg);
	push_msg((uint8_t*)tmp, len);
}

void LLSLAVE::attach(commsCallback func)
{
	callback = func;
}

void LLSLAVE::schedule(void)
{

	switch(rfstate) {
		case INIT:
			if(debug) Serial.print("i");
		case WAITING:
			if(debug) Serial.print("w");
			if(dgram->available()) {			//(0) doesn;t work?
			//~ if(dgram->waitAvailableTimeout(1)) {			//(0) doesn;t work?
				len = MAX_MSG_SIZE;
				if(dgram->recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
					if(debug) Serial.print("r");
					if( (((last_id+1)&0xff) != id) && (rfstate!=INIT)) {
						if(last_id == id) {				//duplicate (or 256 lost..)
							duplicate++;
						}
						else {							//missed
							missed = missed + (((uint16_t)id+0xff)-last_id)&0xff;	//wrap!
						}
						last_id = last_id;	//resent last.
						rfstate = REPEAT;		//REPEAT state
					}
					else {
						last_id = id;
						rfstate = REPLY;		//reply state
					}
					if((callback != NULL) && (len != 0)){
						if(debug) Serial.print("d");
						(*callback)(recv_buf, len);
					}
				}
				else {
					Serial.println("Receive error");
				}
			}
			break;
		case REPLY:
			if(debug) Serial.print("y");
			//copy next message to send
			if(queue_len()){
				if(debug) Serial.print("q");
				len = pop_msg(send_buf);
			}
			else {
				//~ delay(20);	//pong delay while nothing to send
				len = 0;
			}
		case REPEAT:				//repeat last message
			rfstate = SENDING;		//send message
		case SENDING:				//watch fallthrough from RESPONSE
			if(debug) Serial.print("s");
			if(dgram->waitPacketSent(1)) {	//ensure we can send without blocking FIXME:timeout = 0 fails
				if(debug) Serial.print("!");
				dgram->setHeaderId(last_id);
				dgram->setHeaderFlags(queue_len());
				if(!dgram->sendto(send_buf, len, BASE_ADDRESS))
					Serial.println("Sent error");
				dgram->waitPacketSent();	//wait until send
				dgram->available();			//immidiatly to receive, after send() in idle
				rfstate = WAITING;
			}
			else {
				would_block++;
			}
			break;
		default:
			Serial.println("HELP");
			break;
	}
}

uint8_t LLSLAVE::queue_len(void)
{
	return ((head+MSG_BUF_SIZE)-tail)&BUF_SIZE_MASK;
}

bool LLSLAVE::push_msg(uint8_t *msg, uint8_t len) {
	if(((head + 1)&BUF_SIZE_MASK) == tail) {
		Serial.println("Buffer full");
		return false;	//buffer full
	}
	head = ((head + 1)&BUF_SIZE_MASK);
	memcpy(&mesg_buf[head][1], msg, len);	
	mesg_buf[head][0] = len;
}

uint8_t LLSLAVE::pop_msg(uint8_t *msg) {
	uint8_t len;

	if(head == tail)
		return 0;
	len = mesg_buf[tail][0];
	memcpy(msg, &mesg_buf[tail][1], len);	
	tail = (tail + 1)&BUF_SIZE_MASK;
	return len;
}

void LLSLAVE::printmsg(uint8_t *msg)
{
	Serial.print("Received message for : ");
	Serial.print(to);
	Serial.print(" from : ");
	Serial.print(from);
	Serial.print(" with id : ");
	Serial.print(id);
	Serial.print(" and flags : ");
	Serial.println(flags, BIN);
}
void LLMASTER::printmsg(uint8_t *msg)
{
	Serial.print("Received message for : ");
	Serial.print(to);
	Serial.print(" from : ");
	Serial.print(from);
	Serial.print(" with id : ");
	Serial.print(id);
	Serial.print(" and flags : ");
	Serial.println(flags, BIN);
}

LLMASTER::LLMASTER(RHDatagram* driver){
	dgram = driver;
	rfstate = INIT;
	callback = NULL;
}

LLMASTER::~LLMASTER(){};

bool LLMASTER::send(uint8_t *msg, uint8_t len)
{
	send_buf[0] = len;
	memcpy(&send_buf[1], msg, len);
}

void LLMASTER::attach(commsCallback func)
{
	callback = func;
}

void LLMASTER::schedule(void)
{
	int8_t slen;

	switch(rfstate) {
		case INIT:
			if(debug) Serial.print("i");
			id = 0;
		case POLL:
			if(debug) Serial.print("p");
			dgram->setHeaderId(seq);		//one car only for now
			if(queue_len()){
				slen = pop_msg(send_buf);
				if(debug) Serial.print("q");				
			}
			else {
				slen = 0;
				//~ delay(10);	//poll delay while nothing to send
			}
			if(!dgram->sendto(&send_buf[1], slen, CAR_ADDRESS)) {
				if(debug) Serial.print("X");
			}
			dgram->waitPacketSent();	//wait until send
			dgram->available();			//immidiatly to receive, after send() in idle
			reply_timer = millis();
			rfstate = WAITING;
			break;
		case WAITING:
			if(debug) Serial.print("w");
			//~ if (dgram->available()) {
			if(dgram->waitAvailableTimeout(1)) {
				len = MAX_MSG_SIZE; 	
				if(dgram->recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
					if(debug) Serial.print("r");
					if((callback != NULL) && (len != 0)) {
						if(debug) Serial.print("d");
						(*callback)(recv_buf, len);
					}
					seq++;	//next
					msg_count++;
				}
				//~ else
					//~ Serial.println("receive error");
				rfstate = POLL;
			}
			else if((millis() - reply_timer) > reply_timeout ) {
				if(debug) Serial.print("t");
				timeout++;
				//~ Serial.print("timeout : ");
				//~ Serial.println(seq);
				rfstate = POLL;
			}
			break;
		default:
//			Serial.println("help");	
			break;
	}	
}


uint8_t LLMASTER::queue_len(void)
{
	return ((head+MSG_BUF_SIZE)-tail)&BUF_SIZE_MASK;
}

bool LLMASTER::push_msg(uint8_t *msg, uint8_t len) {
	if(((head + 1)&BUF_SIZE_MASK) == tail) {
		return false;	//buffer full
	}
	head = ((head + 1)&BUF_SIZE_MASK);
	memcpy(&mesg_buf[head][1], msg, len);	
	mesg_buf[head][0] = len;
}

uint8_t LLMASTER::pop_msg(uint8_t *msg) {
	uint8_t len;

	if(head == tail)
		return 0;
	len = mesg_buf[tail][0];
	memcpy(msg, &mesg_buf[tail][1], len);	
	tail = (tail + 1)&BUF_SIZE_MASK;
	return len;
}
