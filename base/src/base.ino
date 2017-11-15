/*
 * This is the base software
 * The base is responsible for all the RF communication with the cars
 * The radios are half duplex. The base will continiously poll the cars
 * for data and keep the link alive. The cars all have a unique address
 * also a broadcast adress is available.
 * The base also comminicates with the control software voer the serial
 * port
 * The base has very little(no) knowledge about the application. It just
 * passes messages back and forth
 */
#include <SPI.h>
#include <RH_RF69.h>
#include <RHDatagram.h>
#include <llcom.h>
#include <sframe.h>

#define BASE_ADDRESS (0)
#define CAR_ADDRESS (1)

//setup radio
RH_RF69 rf69(4, 2); // For RF69 on MoteinoMEGA
//Unreliable addressable datagrams
RHDatagram dgram(rf69, BASE_ADDRESS);
//Low latency master
LLMASTER llm(&dgram);

SFRAME sframe;

void setup() 
{
	Serial.begin(38400);	//higher baudrates cause errors
//	Serial.setTimeout(0);
	if (!rf69.init())
		Serial.println("init failed");
	if (!dgram.init())
		Serial.println("dgram init failed");


	//~ rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250); 
	rf69.setModemConfig(RH_RF69::FSK_Rb19_2Fd38_4); 
	//~ rf69.setModemConfig(RH_RF69::OOK_Rb4_8Bw9_6); 
	if (!rf69.setFrequency(915.3))
		Serial.println("setFrequency failed");
	rf69.setTxPower(14);

	//encryption key
	uint8_t key[] = {	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
						0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
//	rf69.setEncryptionKey(key);		//disabled, maybe faster?
	dgram.setHeaderFrom(BASE_ADDRESS);
	llm.attach(rfrecv);
	sframe.attach(handle_msg);
}

void rfrecv(uint8_t *msg, uint8_t len)
{
	//forward to serial host
	sframe.sendframe(msg, len);
}

void handle_msg(uint8_t *msg, uint8_t len)
{
	//pong back over serial
	sframe.sendframe(msg, len);

	//forward to RF
	llm.push_msg(msg, len);
}

void loop()
{
	llm.schedule();
	sframe.schedule();
}
