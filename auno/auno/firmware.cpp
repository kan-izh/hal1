#include "Arduino.h"
#include "HardwareSerial.h"
#include "Rf24Stream.h"
#include "rf-settings.h"
#include "SignalPublisher.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/auno/firmware.hex

const unsigned long interval = 1000;
unsigned long time = 0;
const unsigned long ackTimeout = 1000;

int main()
{
	init();

	RF24 radio(RF_cepin, RF_cspin);
	radio.begin();
	radio.setAutoAck(true);
	radio.setRetries(15, 150);
	radio.setChannel(RF_channel);
	radio.setPALevel(RF24_PA_MAX);
	radio.openWritingPipe(RF_address_write);
	radio.openReadingPipe(1, RF_address_read);
	radio.startListening();

	Rf24Stream rf24Stream(&radio);
	SignalPublisher signalPublisher(&rf24Stream);
    uint8_t counter=0;
	Serial.begin(115200);
	for(;;)
	{
		unsigned long currentMillis = millis();
		if(currentMillis - time > interval)
		{
			time = currentMillis;
			Serial.println("sending");
            const unsigned long startSending = millis();
			//signalPublisher.send();
			rf24Stream.write(counter++);
			rf24Stream.flush();
			Serial.print("sent in ");
            Serial.print(millis() - startSending);
            Serial.println("ms");
			const unsigned long startWaiting = millis();
			bool timedOut = false;
			while(!radio.available() && !timedOut)
			{
				timedOut = millis() - startWaiting > ackTimeout;
			}
			Serial.print("waited ");
			Serial.print(millis() - startWaiting);
			Serial.println("ms");
			if(!timedOut)
			{
				Serial.print("Data ");
				while(radio.available())
				{
					Serial.print(rf24Stream.read(), HEX);
				}
				Serial.println();
			}
            else
            {
                Serial.println("Timed out");
            }
		}
	}
	return 0;
}
