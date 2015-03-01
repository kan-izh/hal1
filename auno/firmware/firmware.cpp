#include <RF24.h>
#include <RPi/RF24/RF24.h>
#include "Arduino.h"
#include "HardwareSerial.h"
#include "Rf24Stream.h"
#include "rf-settings.h"
#include "RingBufferPublisher.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/firmware/firmware.hex
const int temperatureSensorPin = 0;

typedef RingBufferPublisher<16> RfPublisher;

struct RF24Output : RingBufferOutput
{

    RF24 &radio;

    RF24Output(RF24 &radio) : radio(radio)
    {
    }

    virtual void write(uint8_t const *buf, uint8_t len)
    {
        bool written = radio.writeFast(buf, len);
        Serial.println(written ? "written ok" : "not written");
    }
};

const unsigned long heartbeatIntervalMs = 250;
unsigned long timeMs = 0;

void schedule(RF24 &radio, RfPublisher &publisher);

bool work = true;

int main()
{
	init();
    Serial.begin(115200);

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

    RF24Output rf24Output(radio);
    RfPublisher ringBufferPublisher(&rf24Output);
    Serial.println("initialised");
    Serial.flush();
    while(work)
	{
        schedule(radio, ringBufferPublisher);
    }
	return 0;
}

int readTemperature()
{
    unsigned long val = analogRead(temperatureSensorPin);
    return (125 * val * 100) / 256;
}

void schedule(RF24 &radio, RfPublisher &publisher)
{
    unsigned long currentTime = millis();
    if(currentTime - timeMs > heartbeatIntervalMs)
    {
        timeMs = currentTime;
        radio.stopListening();
        *(int *) (publisher.getSendBuffer()) = readTemperature();
        publisher.send();
        radio.startListening();
    }
}
