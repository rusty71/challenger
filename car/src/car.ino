#include <SPI.h>
#include <RH_RF69.h>
#include <RHDatagram.h>
#include <tb6612.h>
#include <blade.h>
#include <adxl345.h>
#include <sfreed.h>
#include <tachometer.h>
#include <llcom.h>

#define STANDALONE 1

#ifdef STANDALONE
#define CAR_SPEED (0)
#define CAR_HIGH_SPEED (0)
#else
#define CAR_SPEED (100)
#define CAR_HIGH_SPEED (200)
#endif //STANALONE

// Singleton instance of the radio driver
RH_RF69 rf69(4, 2); // For MoteinoMEGA https://lowpowerlab.com/shop/moteinomega
//Unreliable addressable datagrams
RHDatagram dgram(rf69, CAR_ADDRESS);
//Singleton motor driver 
TB6612 Motor;
//Singleton blade guide 
BLADE blade;
//Singleton accelerometer
ADXL345 adxl345;
//~ //Singleton Start Finish sensor
SFREED sfreed;
//Singleton Tachometer
TACHOMETER tacho;
//Low latency slave
LLSLAVE lls(&dgram);

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
//	rf69.setEncryptionKey(key);

	//setup motor
	Motor.coast(0);
	Motor.move(0,1);
	//blade
	blade.begin();
	adxl345.begin();
	sfreed.begin();
	//attach callback at every SF
	sfreed.attach(SF_DETECT);

	tacho.begin(1);		//every 18 ticks (~2mm/tick) increment segment position

	lls.attach(recv);

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
	message_t	type;
	uint16_t	segment;
	int16_t		blade_pos;
	int16_t		xyz[3];
	uint32_t	ticks;		//total ticks elapsed
	uint32_t	millis;		//total number of milliseconds
} comms_t;
comms_t comms;


void recv(uint8_t *msg, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++) {
		Serial.print(msg[i], HEX);
		Serial.print(" ");
	}
	Serial.println("");
}

uint8_t test_msg[5]={1,2,3,4,5};	

uint8_t i=0;

int16_t speed=CAR_SPEED;
void loop()
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

	//~ rf69.send((uint8_t*)&comms, sizeof(comms));
	//~ rf69.waitPacketSent();
	comms.type = SEQ;
	lls.schedule();	//wait for next segment
	lls.send((uint8_t*)&comms, sizeof(comms));
	lls.schedule();	//wait for next segment

#ifdef STANDALONE
	if(tacho.get() > 40){
		tacho.sim(0);
		SF_DETECT();
	}
	//~ tacho.sim(tacho.get()+1);
	//~ delay(10);
#endif


	while(comms.segment == tacho.get())
		lls.schedule();	//wait for next segment
	//~ lls.schedule();	//wait for next segment
	//~ lls.schedule();	//wait for next segment
}
