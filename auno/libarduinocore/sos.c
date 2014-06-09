#include "sos.h"
#include "Arduino.h"

static void showSignal(int dash)
{
	digitalWrite(LED_BUILTIN, HIGH);
	delay(dash ? 400 : 100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(dash ? 200 : 100);
}

void showSos(void)
{
	pinMode(LED_BUILTIN, OUTPUT);
	showSignal(0);
	showSignal(0);
	showSignal(0);
	delay(200);
	showSignal(1);
	showSignal(1);
	showSignal(1);
	delay(200);
	showSignal(0);
	showSignal(0);
	showSignal(0);
	delay(1000);
}
