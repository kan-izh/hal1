#include <RF24.h>
#include "Arduino.h"
#include "HardwareSerial.h"
#include "Rf24Stream.h"
#include "rf-settings.h"
#include "../../conf/messages.h"
#include "RingBufferPublisher.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/firmware/firmware.hex
typedef RingBufferPublisher<16, RF_PAYLOAD_SIZE> RfPublisher;

struct RF24Output : RingBufferOutput
{

    RF24 &radio;

    RF24Output(RF24 &radio) : radio(radio)
    {
    }

    virtual void write(uint8_t const *buf, uint8_t len)
    {
        radio.stopListening();
        bool written = radio.writeBlocking(buf, len, 2000);
        bool txen = radio.txStandBy();
        radio.startListening();
        Serial.print(written ? "written ok" : "not written. ");
        Serial.println(txen ? "txen ok" : "not txen");
    }
};

const unsigned long heartbeatIntervalMs = 1000;
unsigned long timeMs = 0;

void schedule(RF24 &radio, RfPublisher &publisher);

void sendAnalogRead(RfPublisher &publisher, const uint8_t i);

bool work = true;

int main()
{
	init();
    Serial.begin(115200);

	RF24 radio(RF_cepin, RF_cspin);
	radio.begin();
	radio.setAutoAck(true);
    radio.setChannel(RF_channel);
	radio.setPALevel(RF24_PA_MAX);
    radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_address_write);
	radio.openReadingPipe(1, RF_address_read);
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

void schedule(RF24 &radio, RfPublisher &publisher)
{
    unsigned long currentTime = millis();
    if(currentTime - timeMs > heartbeatIntervalMs)
    {
        timeMs = currentTime;
		sendAnalogRead(publisher, TEMPERATURE_SENSOR);
    }
}

void sendAnalogRead(RfPublisher &publisher, const uint8_t pin)
{
	const uint16_t value = (uint16_t) analogRead(pin);
	publisher.getSendBuffer()
				.put(MESSAGE_ANALOG_READ)
				.put(pin)
				.put(value);
	publisher.send();
}
