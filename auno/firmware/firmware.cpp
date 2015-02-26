#include "Arduino.h"
#include "HardwareSerial.h"
#include "Rf24Stream.h"
#include "rf-settings.h"
#include "RingBufferPublisher.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/firmware/firmware.hex

struct RF24Output : RingBufferOutput
{
	virtual void write(uint8_t const *buf, uint8_t len)
	{
	}
} rf24Output;

RingBufferPublisher<128> ringBufferPublisher(&rf24Output);

const unsigned long interval = 1000;
unsigned long time = 0;

void doStuff(RF24 &radio, uint8_t &counter);

const unsigned long ackTimeout = 1000;

int main()
{
	init();

	RF24 radio(RF_cepin, RF_cspin);
	radio.begin();
	radio.setAutoAck(true);
	radio.setRetries(15, 15);
	radio.setChannel(RF_channel);
	radio.setPALevel(RF24_PA_MAX);
	radio.openWritingPipe(RF_address_write);
	radio.openReadingPipe(1, RF_address_read);
    radio.enableAckPayload();
	radio.startListening();

    uint8_t counter=0;
    radio.writeAckPayload(1, &counter, sizeof(counter));
	Serial.begin(115200);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	for(;;)
	{
		unsigned long currentMillis = millis();
		if(currentMillis - time > interval)
		{
			time = currentMillis;

        }
        doStuff(radio, counter);
    }
#pragma clang diagnostic pop
	return 0;
}

void doStuff(RF24 &radio, uint8_t &counter)
{
    if(radio.available()) {
        int data;
        radio.read(&data, sizeof(data));
        Serial.print("received ");
        Serial.print(data);
        Serial.println();
        Serial.flush();
        radio.writeAckPayload(1, &data, sizeof(data));
    }
}
/*
void doStuff(RF24 &radio, uint8_t &counter) {
    Serial.println("sending");
    const unsigned long startSending = millis();
    //signalPublisher.send();
    radio.stopListening();
    bool written = radio.write(&counter, sizeof(counter));
    radio.startListening();
    ++counter;
    Serial.print("sent with ");
    Serial.print(written ? "SUCCESS" : "FAILURE");
    Serial.print(" in ");
    Serial.print(millis() - startSending);
    Serial.println("ms");
    Serial.flush();
    const unsigned long startWaiting = millis();
    bool timedOut = false;
    while(!timedOut)
			{
                if(radio.available())
                {
                    Serial.print("Data ");
                    int response;
                    radio.read(&response, sizeof(response));
                    Serial.print(response, HEX);
                    Serial.println();
                    break;
                }
				timedOut = millis() - startWaiting > ackTimeout;
			}
    Serial.print(timedOut ? "timed out " : "waited ");
    Serial.print(millis() - startWaiting);
    Serial.println("ms");
    Serial.flush();
}
*/
