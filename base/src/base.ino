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

uint8_t i=0;
void loop()
{
	Serial.println("Sending..");
	dgram.setHeaderId(i++);
	//~ while(!dgram.waitPacketSent(1))
		//~ ;
	dgram.waitPacketSent();
	dgram.sendto(send_buf, 20, RH_BROADCAST_ADDRESS);
	if(dgram.waitAvailableTimeout(30)) { 	
		if(dgram.recvfrom (recv_buf, &len, &from, &to, &id, &flags)) {
			//~ Serial.print("Received message for : ");
			//~ Serial.print(to);
			//~ Serial.print(" from : ");
			//~ Serial.print(from);
			//~ Serial.print(" with id : ");
			//~ Serial.print(id);
			//~ Serial.print(" and flags : ");
			//~ Serial.println(flags, BIN);
		}
	}
	else
		Serial.println("Timeout...");
}
