#include <SPI.h>
#include <RH_RF69.h>
#include <RHDatagram.h>
#include <tb6612.h>
#include <blade.h>
#include <adxl345.h>
#include <sfreed.h>
#include <tachometer.h>

#define STANDALONE 1

#ifdef STANDALONE
#define CAR_SPEED (0)
#define CAR_HIGH_SPEED (0)
#else
#define CAR_SPEED (100)
#define CAR_HIGH_SPEED (200)
#endif //STANALONE

#define BASE_ADDRESS (0)
#define CAR_ADDRESS (1)



// Singleton instance of the radio driver
RH_RF69 rf69(4, 2); // For MoteinoMEGA https://lowpowerlab.com/shop/moteinomega
//Unreliable addressable datagrams
RHDatagram dgram(rf69, CAR_ADDRESS);

//Singleton motor driver 
TB6612 Motor;
//Singleton blade guide 
BLADE blade;
//~ //Singleton accelerometer
ADXL345 adxl345;
//~ //Singleton Start Finish sensor
SFREED sfreed;
//~ //Singleton Tachometer
TACHOMETER tacho;


//~ //called from interrupt dont delay()
void SF_DETECT(void)
{
	//reset position on SF detect
	blade.reset();
	tacho.reset();
}

void setup() 
{
	uint16_t sfcount;

	Serial.begin(115200);
	Serial.println("calibrating...");
	Motor.coast(0);

	//setup telemetry radio
	if (!rf69.init())
		Serial.println("init failed");
	if (!dgram.init())
		Serial.println("dgram init failed");

	rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250); 
	if (!rf69.setFrequency(915.3))
		Serial.println("setFrequency failed");
	//~ // range 14 to 20. Too high doesn't mean better reception for short range
	rf69.setTxPower(14);
	dgram.setHeaderFrom(CAR_ADDRESS);

	//encryption key
	uint8_t key[] = { 	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
						0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	rf69.setEncryptionKey(key);

	//setup motor
	Motor.coast(0);
	Motor.move(0,1);
	//blade
	blade.begin();
	adxl345.begin();
	sfreed.begin();
	//attach callback at every SF
	sfreed.attach(SF_DETECT);

	tacho.begin(36);		//every 18 ticks (~2mm/tick) increment segment position

	//current lap count (should be zero)
	sfcount = sfreed.get();
	//start car and wait for SF
	Motor.move(CAR_SPEED,1);
	//wait for start finish
	
#ifndef STANDALONE
	while(sfcount == sfreed.get())
		;
#endif
	//~ Serial.println("car started");
	//~ Serial.println("start motor");
}

typedef struct {
	uint16_t	segment;
	int16_t		blade_pos;
	int16_t		xyz[3];
	uint32_t	ticks;		//total ticks elapsed
	uint32_t	millis;		//total number of milliseconds
} comms_t;
comms_t comms;

int16_t speed=CAR_SPEED;
void loop_seq()
{
	
	//collect data and send
	comms.segment = tacho.get();
	comms.segment = comms.segment*2;
	if( ((comms.segment < 56) && (comms.segment > 35)) || ((comms.segment < 100) && (comms.segment > 80)) || ((comms.segment < 8) && (comms.segment > 0)))
		speed = CAR_HIGH_SPEED;
	else
		speed = CAR_SPEED;

	if((comms.segment < 63) && (comms.segment > 57))
		Motor.brake(1);
	else{
		Motor.move(speed, 1);
	}
	comms.segment = tacho.get();
	
	//~ 40-60
//~ 90-10
	comms.blade_pos = blade.get();
	adxl345.get(comms.xyz);
	comms.ticks = tacho.getticks();
	comms.millis = millis();
	//~ Serial.print(comms.blade_pos);
	//~ Serial.print("\t");
	//~ Serial.print(comms.xyz[0]);
	//~ Serial.print("\t");
	//~ Serial.println(comms.segment);

	rf69.send((uint8_t*)&comms, sizeof(comms));
	//waitPacketSent() not needed?
	rf69.waitPacketSent();

#ifdef STANDALONE
	if(tacho.get() >100){
		tacho.sim(0);
		SF_DETECT();
	}
	tacho.sim(tacho.get()+1);
	delay(10);
#endif

	while(comms.segment == tacho.get());
		;	//wait for next segment

}

#define MAX_MSG_SIZE (64)
#define MSG_BUF_SIZE (8)			//power of 2
#define BUF_SIZE_MASK (0x07)		//power of 2

class CLIENTCOMM { 
public:
	CLIENTCOMM(){
		rfstate = INIT;
	}

	~CLIENTCOMM(){};
	bool send(uint8_t *msg, uint8_t len)
	{
		push_msg(msg, len);
	}

	void schedule(void)
	{

		switch(rfstate) {
			case INIT:
			case WAITING:
				if(dgram.waitAvailableTimeout(1)) {			//(0) doesn;t work?
					len = MAX_MSG_SIZE;
					if(dgram.recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
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
//						printmsg(recv_buf);
						rfstate = REPLY;		//reply state
					}
					else {
						Serial.println("Receive error");
					}
				}
				break;
			case REPLY:
				//copy next message to send
				if(queue_len())
					len = pop_msg(send_buf);
				else
					len = 0;
			case REPEAT:				//repeat last message
				rfstate = SENDING;		//send message
			case SENDING:				//watch fallthrough from RESPONSE
				if(dgram.waitPacketSent(1)) {	//ensure we can send without blocking FIXME:timeout = 0 fails
					dgram.setHeaderId(last_id);
					dgram.setHeaderFlags(queue_len());
					dgram.sendto(send_buf, len, BASE_ADDRESS);
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
	uint8_t head = 0, tail = 0;
	uint8_t queue_len(void)
	{
		return ((head+MSG_BUF_SIZE)-tail)&BUF_SIZE_MASK;
	}

private:
	typedef enum state_t {
							INIT,
							WAITING,
							REPEAT,
							REPLY,
							SENDING
						 };
	state_t rfstate = INIT;
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

	
	bool push_msg(uint8_t *msg, uint8_t len) {
		if(((head + 1)&BUF_SIZE_MASK) == tail) {
			Serial.println("Buffer full");
			return false;	//buffer full
		}
		head = ((head + 1)&BUF_SIZE_MASK);
		memcpy(&mesg_buf[head][1], msg, len);	
		mesg_buf[head][0] = len;
	}

	uint8_t pop_msg(uint8_t *msg) {
		uint8_t len;

		if(head == tail)
			return 0;
		len = mesg_buf[tail][0];
		memcpy(msg, &mesg_buf[tail][1], len);	
		tail = (tail + 1)&BUF_SIZE_MASK;
		return len;
	}

	void printmsg(uint8_t *msg)
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

};

CLIENTCOMM cm;
uint8_t test_msg[5];	

uint8_t i=0;
void loop()
{
	i++;
	//~ Serial.print("Loop..");
	//~ Serial.println(rfstate);
	//~ delay(20);

	//emulate burst sent
	if(i>254) {
		cm.send(test_msg, 4);
		cm.send(test_msg, 4);
		cm.send(test_msg, 4);
		cm.send(test_msg, 4);
		Serial.print(cm.head);
		Serial.print(" : ");
		Serial.print(cm.tail);
		Serial.print(" : ");
		Serial.println(cm.queue_len());
	}	
	cm.schedule();
	cm.schedule();
	//delay(1000);
}
