/*
 * This is the base software
 * The base is responsible for all the RF communication with the cars
 * The radios are half duplex. The base will continiously poll the cars
 * for data and keep the link alive. The cars all have a unique address
 * also a broadcast adress is available.
 * The base also comminicates with the control software voer the serial
 * port using CmdMessenger
 * The base has very little knowledge about the application
 */
 
#include <SPI.h>
#include <RH_RF69.h>
#include <RHDatagram.h>
#include <CmdMessenger.h>

#define BASE_ADDRESS (0)
#define CAR_ADDRESS (1)

//setup radio
RH_RF69 rf69(4, 2); // For RF69 on MoteinoMEGA
//Unreliable addressable datagrams
RHDatagram dgram(rf69, BASE_ADDRESS);


//~ typedef enum state_t {
						//~ WAITING,
						//~ RESPONSE
					 //~ };
//~ state_t rfstate = WAITING;

//host communication
CmdMessenger c = CmdMessenger(Serial,',',';','/');

enum {
    seq_cmd,
};

#define CM_TICK (0.1832)
typedef struct {
	uint16_t	segment;
	int16_t		blade_pos;
	int16_t		xyz[3];
	uint32_t	ticks;		//total ticks elapsed
	uint32_t	millis;		//total number of milliseconds
} comms_t;
comms_t comms;

void setup() 
{
	Serial.begin(115200);
	if (!rf69.init())
		Serial.println("init failed");
	if (!dgram.init())
		Serial.println("dgram init failed");

	rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250); 
	if (!rf69.setFrequency(915.3))
		Serial.println("setFrequency failed");
	rf69.setTxPower(14);

	//encryption key
	uint8_t key[] = {	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
						0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	rf69.setEncryptionKey(key);			
	dgram.setHeaderFrom(BASE_ADDRESS);
}


uint32_t last_ticks = 0;
uint32_t last_millis = millis();
void raceloop()
{
	uint8_t len = sizeof(comms);
	if (rf69.available())
	{
		if (rf69.recv((uint8_t*)(&comms), &len))
		{
			//RH_RF69::printBuffer("received: ", (uint8_t*)(&comms), len);
			//~ Serial.print(comms.segment,DEC);
			//~ Serial.print("\t");
			//~ Serial.print(comms.blade_pos,DEC);
			//~ Serial.print("\t");
			//~ Serial.print(((double)comms.xyz[0])/32.0);
			//~ Serial.print("\t");
			//~ Serial.print(((double)comms.xyz[1])/32.0);
			//~ Serial.print("\t");
			//~ Serial.print(((double)comms.xyz[2])/32.0);
			//~ Serial.print("\t");
			//~ Serial.print(comms.millis);
			//~ Serial.print("\t");
			//~ Serial.print(comms.millis-last_millis);
			//~ Serial.print("\t");
			//~ Serial.print(comms.ticks-last_ticks);
			//~ Serial.print("\t");
			//~ Serial.print((((double)(comms.millis-last_millis)/(double)(comms.ticks-last_ticks))*1000.0));
			//~ Serial.print("\t");
			//~ Serial.println(comms.ticks,DEC);
			last_ticks = comms.ticks;
			last_millis = comms.millis;
			c.sendCmdStart(seq_cmd);
			c.sendCmdBinArg(comms.segment);
			c.sendCmdBinArg(comms.blade_pos);
			c.sendCmdBinArg(comms.xyz[0]);
			c.sendCmdBinArg(comms.xyz[1]);
			c.sendCmdBinArg(comms.xyz[2]);
			c.sendCmdBinArg(comms.millis);
			c.sendCmdBinArg(comms.ticks);
			c.sendCmdEnd();		
		}
		else
		{
			Serial.println("recv failed");
		}
	}
}

uint8_t send_buf[64];
uint8_t recv_buf[64];
uint8_t len, to, from, id, flags;
uint8_t seq = 0;
uint32_t reply_timer = 0;
uint16_t timeout = 0;

uint8_t i=0;

typedef enum state_tx_t {
						INIT,
						POLL,
						WAITING,
						REPLY,
						SENDING
					 };
state_tx_t rfstate = INIT;
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

uint16_t reply_timeout = 200;	//ms
uint16_t msg_count = 0;
uint16_t loop_count = 0;

void loop()
{
	if(loop_count++%1000 == 0)
		Serial.println(msg_count);
	//~ delay(20);
	//~ Serial.print("Loop..");
	//~ Serial.println(rfstate);
//	dgram.setHeaderId(i++);
	//~ while(!dgram.waitPacketSent(1))
		//~ ;
	switch(rfstate) {
		case INIT:
			id = 0;
		case POLL:
			dgram.setHeaderId(seq);		//one car only for now
			dgram.sendto(&send_buf[1], send_buf[0], CAR_ADDRESS);
			send_buf[0] = 0;	//mark sent
			reply_timer = millis();
			rfstate = WAITING;
			break;
		case WAITING:
			if(dgram.waitAvailableTimeout(1)) {
				 	
				if(dgram.recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
					//printmsg(recv_buf);
					seq++;	//next
					msg_count++;
				}
				else
					Serial.println("receive error");
				rfstate = POLL;
			}
			else if((millis() - reply_timer) > reply_timeout ) {
				timeout++;
				Serial.println(seq);
				rfstate = POLL;
			}
			break;
		default:
			Serial.println("help");	
			break;
		}
}
