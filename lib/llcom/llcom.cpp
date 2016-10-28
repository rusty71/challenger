#include "llcom.h"
#include "RHReliableDatagram.h"

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
	uint8_t len = strlen(msg);
	
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
		case WAITING:
			if(dgram->waitAvailableTimeout(1)) {			//(0) doesn;t work?
				len = MAX_MSG_SIZE;
				if(dgram->recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
					if( (((last_id+1)&0xff) != id) && (rfstate!=INIT)) {
						if(last_id == id) {				//duplicate (or 256 lost..)
							duplicate++;
						}
						else {							//missed
							missed = missed + (((uint16_t)id+0xff)-last_id)&0xff;	//wrap!
						}
						last_id = last_id;	//resent last.
					}
					else {
						last_id = id;
					}
					if((callback != NULL) && (len != 0))
						(*callback)(recv_buf, len);
					rfstate = REPLY;		//reply state
				}
				else {
					Serial.println("Receive error");
				}
			}
			break;
		case REPLY:
			//copy next message to send
			if(queue_len()){
				len = pop_msg(send_buf);
			}
			else {
				len = 0;
			}
		case REPEAT:				//repeat last message
			rfstate = SENDING;		//send message
		case SENDING:				//watch fallthrough from RESPONSE
			if(dgram->waitPacketSent(1)) {	//ensure we can send without blocking FIXME:timeout = 0 fails
				dgram->setHeaderId(last_id);
				dgram->setHeaderFlags(queue_len());
				if(!dgram->sendto(send_buf, len, RH_BROADCAST_ADDRESS))
					Serial.println("Sent error");
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
	switch(rfstate) {
		case INIT:
			id = 0;
		case POLL:
			dgram->setHeaderId(seq);		//one car only for now
			dgram->sendto(&send_buf[1], send_buf[0], RH_BROADCAST_ADDRESS);
			send_buf[0] = 0;	//mark sent, mailbox byte @send_buf[0]
			reply_timer = millis();
			rfstate = WAITING;
			break;
		case WAITING:
			if(dgram->waitAvailableTimeout(1)) {
				len = MAX_MSG_SIZE; 	
				if(dgram->recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
					if((callback != NULL) && (len != 0))
						(*callback)(recv_buf, len);
					seq++;	//next
					msg_count++;
				}
				//~ else
					//~ Serial.println("receive error");
				rfstate = POLL;
			}
			else if((millis() - reply_timer) > reply_timeout ) {
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

