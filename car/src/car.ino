#include <SPI.h>
#include <RH_RF69.h>
#include <tb6612.h>
#include <blade.h>
#include <adxl345.h>
#include <sfreed.h>
#include <tachometer.h>

#define CAR_SPEED (80)

// Singleton instance of the radio driver
RH_RF69 rf69(4, 2); // For MoteinoMEGA https://lowpowerlab.com/shop/moteinomega
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
	rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250); 
	if (!rf69.setFrequency(915.3))
		Serial.println("setFrequency failed");
	//~ // range 14 to 20. Too high doesn't mean better reception for short range
	rf69.setTxPower(14);
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

	tacho.begin(18);		//every 18 ticks (~2mm/tick) increment segment position

	//current lap count (should be zero)
	sfcount = sfreed.get();
	//start car and wait for SF
	Motor.move(CAR_SPEED,1);
	//wait for start finish
	while(sfcount == sfreed.get())
		;
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

void loop()
{
	//collect data and send
	comms.segment = tacho.get();
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

	while(comms.segment == tacho.get());
		;	//wait for next segment

}

