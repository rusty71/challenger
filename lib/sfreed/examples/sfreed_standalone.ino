const byte SF_PIN = 11;

#define DEBOUNCE_MS 100

extern "C"
{
	// callback typedef
	typedef void(*sfCallback) (void);
}

//only one callback possible
sfCallback callback = NULL;

//interrupt shared variables need to be volatile
volatile uint16_t sf_count = 0;
volatile long long debounce = 0;

void SF_INT(void) {
	if((millis() - debounce)>DEBOUNCE_MS) {
		sf_count++;
		if (callback != NULL) (*callback)();
		debounce = millis();
	}
	//else debounce
}

void attach(sfCallback func)
{
	callback = func;
}

//callback called from INT1 ISR. Should be handled as fast as possible
//No delay() and Serial()
void SF(void) {
	Serial.print("detect : ");
	Serial.println(sf_count);
}	

void setup()
{
	Serial.begin(115200);

	Serial.println("Start Finish Reed switch test");
	delay(1000);
	pinMode(SF_PIN, INPUT_PULLUP);
	attachInterrupt(1, SF_INT, FALLING);
	attach(SF);
}

uint16_t prev_count = 0;

void loop() {
	Serial.println("waiting for SF");
	delay(1000);
}
