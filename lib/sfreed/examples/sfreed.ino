#include <sfreed.h>

SFREED sf;
//callback called from INT1 ISR. Should be handled as fast as possible
//No delay() and Serial()
void SF(void) {
	Serial.print("detect : ");
	Serial.println(sf.get());
}	

void setup()
{
	Serial.begin(115200);
	Serial.println("Start Finish Reed switch test");
	delay(1000);
	sf.begin();
	sf.attach(SF);
}

uint16_t prev_count = 0;

void loop() {
	Serial.println("waiting for SF");
	delay(1000);
}
