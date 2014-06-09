#include "Arduino.h"
#include "HardwareSerial.h"
#include "SignalPublisher.h"
#include "sos.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/auno/firmware.hex

SignalPublisher signalPublisher(&Serial);

unsigned long interval = 2000;
unsigned long time = 0;

int main()
{
	init();
	Serial.begin(115200);

	pinMode(LED_BUILTIN, OUTPUT);
	for(;;)
	{
		unsigned long currentMillis = millis();
		//if(currentMillis - time > interval)
		{
			time = currentMillis;
			signalPublisher.send();
		}
	}
	return 0;
}
