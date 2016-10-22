#include <SPI.h>
#include <RH_RF69.h>

RH_RF69 rf69(4, 2); // For RF69 on MoteinoMEGA

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
}


uint32_t last_ticks = 0;
uint32_t last_millis = millis();
void loop()
{
	uint8_t len = sizeof(comms);
	if (rf69.available())
	{
		if (rf69.recv((uint8_t*)(&comms), &len))
		{
			//RH_RF69::printBuffer("received: ", (uint8_t*)(&comms), len);
			Serial.print(comms.segment,DEC);
			Serial.print("\t");
			Serial.print(comms.blade_pos,DEC);
			Serial.print("\t");
			Serial.print(((double)comms.xyz[0])/32.0);
			Serial.print("\t");
			Serial.print(((double)comms.xyz[1])/32.0);
			Serial.print("\t");
			Serial.print(((double)comms.xyz[2])/32.0);
			Serial.print("\t");
			Serial.print(comms.millis);
			Serial.print("\t");
			Serial.print(comms.millis-last_millis);
			Serial.print("\t");
			Serial.print(comms.ticks-last_ticks);
			Serial.print("\t");
			Serial.print((((double)(comms.millis-last_millis)/(double)(comms.ticks-last_ticks))*1000.0));
			Serial.print("\t");
			Serial.println(comms.ticks,DEC);
			last_ticks = comms.ticks;
			last_millis = comms.millis;
		}
		else
		{
			Serial.println("recv failed");
		}
	}
}

