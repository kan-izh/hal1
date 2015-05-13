#include <cstdlib>
#include <iostream>

#include <RF24.h>
#include "../conf/rf-comm.h"
#include "RingBufferSubscriber.h"
#include "RingBufferPublisher.h"
#include "../conf/messages.h"

typedef RingBufferSubscriber<RF_PAYLOAD_SIZE> Subscriber;
typedef RingBufferPublisher<1024, RF_PAYLOAD_SIZE> RfPublisher;


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
		printf(written ? "written ok. " : "not written. ");
		printf(txen ? "txen ok.\n" : "not txen.\n");
		fflush(stdout);
	}
};

uint32_t convertTemperature(const uint16_t value)
{
	return (uint32_t(125) * value * 1000) / 256;
}

void display(uint32_t temperatureInMilliC);

struct SubscriberHandler : Subscriber::Handler
{
	RfPublisher &publisher;
	RF24 &rf24;
	SubscriberHandler(RfPublisher &publisher, RF24 &rf24)
			: publisher(publisher)
			, rf24(rf24)
	{ }

	void messageAnalogRead(const uint8_t &pin, const uint16_t &value)
	{
		switch (pin)
		{
			case TEMPERATURE_SENSOR:
			{
				uint32_t temperatureInMilliC = convertTemperature(value);
				display(temperatureInMilliC);
				break;
			}
			default:
				break;
		}
	}

	virtual void handle(uint32_t messageId, uint8_t contentId, Subscriber::PayloadAccessor &input)
	{
		switch (contentId)
		{
			case MESSAGE_ANALOG_READ:
			{
				const uint8_t &pin = input.template take<uint8_t>();
				const uint16_t &value = input.template take<uint16_t>();
				messageAnalogRead(pin, value);
				break;
			}
			default:
				break;
		}
	}

	virtual void handleNak(uint32_t hwm, uint32_t sequence)
	{
		while(rf24.available())
		{
			uint8_t b;
			rf24.read(&b, sizeof(b));
		}
		printf("\n[%d] sending ack for %d, got %d\n", millis(), hwm, sequence);
		fflush(stdout);
		publisher.sendNak(hwm);
	}

	virtual void recover(uint32_t hwm)
	{
		printf("\n[%d] Recovered from %d\n", millis(), hwm);
		fflush(stdout);
	}

	virtual void nak(const uint32_t &subscriberHighWatermark)
	{
	}
};

bool work = true;

void loop(RF24 radio, Subscriber &subscriber);

int main(int argc, char *argv[])
{
	RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);
	radio.begin();
	radio.setPayloadSize(RF_PAYLOAD_SIZE);
	radio.setChannel(RF_COMM_CHANNEL_ARDUINO_TO_RPI);
	radio.setPALevel(RF24_PA_MAX);
	radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_COMM_ADDRESS_RPI_TO_ARDUINO);
	radio.openReadingPipe(1, RF_COMM_ADDRESS_ARDUINO_TO_RPI);
	radio.startListening();
	radio.printDetails();
	fflush(stdout);

	RF24Output rf24Output(radio);
	RfPublisher publisher(&rf24Output);
	SubscriberHandler handler(publisher, radio);
	Subscriber subscriber(&handler);

	while(work)
	{
		loop(radio, subscriber);
	}
	return 0;
}

const int tempHistorySize = 1024;
int tempHistory[tempHistorySize] = {};
int tempHistoryPos = 0;
int pulse=0;

void display(uint32_t temperatureInMilliC)
{
	tempHistory[tempHistoryPos] = temperatureInMilliC;
	tempHistoryPos++;
	if(tempHistoryPos >= tempHistorySize)
		tempHistoryPos = 0;
	int avg = 0, cnt = 0;
	for (int i = 0; i < tempHistorySize; ++i)
	{
		int t = tempHistory[i];
		if(t != 0)
		{
			++cnt;
			avg += t;
		}
	}
	avg /= cnt;
	printf("average(%d): %6.2f, current: %6.2f\r", cnt, avg / 1000., temperatureInMilliC / 1000.);
	fflush(stdout);
}


void loop(RF24 radio, Subscriber &subscriber)
{
	while (radio.available())
	{
		pulse = 0;
		radio.read(subscriber.getBuf(), subscriber.getBufSize());
		subscriber.process();
	}
	delay(100);
	if (++pulse % 20 == 0)
	{
		printf("\npulse %d\n", pulse);
		fflush(stdout);
	}
}
