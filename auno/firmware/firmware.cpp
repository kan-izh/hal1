#include <RF24.h>
#include "Arduino.h"
#include "HardwareSerial.h"
#include "Rf24Stream.h"
#include "rf-settings.h"
#include "../../conf/messages.h"
#include "RingBufferSubscriber.h"
#include "RingBufferPublisher.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/firmware/firmware.hex
typedef RingBufferSubscriber<RF_PAYLOAD_SIZE> RfSubscriber;
typedef RingBufferPublisher<128, RF_PAYLOAD_SIZE> RfPublisher;

struct SubscriberHandler : RfSubscriber::Handler
{
	RfPublisher &publisher;

	SubscriberHandler(RfPublisher &publisher, RF24 &rf24)
			:publisher(publisher)
	{ }

	virtual void handle(uint32_t messageId, uint8_t contentId, RfSubscriber::PayloadAccessor &input)
	{
	}

	virtual void handleNak(uint32_t hwm, uint32_t sequence)
	{
	}

	virtual void recover(uint32_t hmw)
	{
	}

	virtual void nak(const uint32_t &subscriberHighWatermark)
	{
		const uint32_t hwm = publisher.getHighWatermark();
		Serial.print(hwm);
		Serial.print(", naked=");
		Serial.println(uint32_t(subscriberHighWatermark));
		publisher.nak(subscriberHighWatermark);
	}
};

struct RF24Output : RingBufferOutput
{

    RF24 &radio;

    RF24Output(RF24 &radio) : radio(radio)
    {
    }

    virtual void write(uint8_t const *buf, uint8_t len)
    {
        radio.stopListening();
        radio.writeBlocking(buf, len, 2000);
        radio.txStandBy();
        radio.startListening();
    }
};

const unsigned long heartbeatIntervalMs = 50;
unsigned long timeMs = 0;

void schedule(RF24 &radio, RfPublisher &publisher);
void listenRf(RF24 &radio, RfSubscriber &subscriber);

void sendAnalogRead(RfPublisher &publisher, const uint8_t i);

bool work = true;

int main()
{
	init();
    Serial.begin(115200);

	RF24 radio(RF_cepin, RF_cspin);
	radio.begin();
	radio.setPayloadSize(RF_PAYLOAD_SIZE);
	radio.setAutoAck(true);
    radio.setChannel(RF_channel);
	radio.setPALevel(RF24_PA_MAX);
    radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_address_write);
	radio.openReadingPipe(1, RF_address_read);
	radio.startListening();

    RF24Output rf24Output(radio);
    RfPublisher ringBufferPublisher(&rf24Output);
	SubscriberHandler subscriberHandler(ringBufferPublisher, radio);
	RfSubscriber subscriber(&subscriberHandler);
    Serial.println("initialised");
    Serial.flush();
    while(work)
	{
		listenRf(radio, subscriber);
		schedule(radio, ringBufferPublisher);
    }
	return 0;
}

void listenRf(RF24 &radio, RfSubscriber &subscriber)
{
	while(radio.available())
	{
		radio.read(subscriber.getBuf(), subscriber.getBufSize());
		subscriber.process();
	}
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
