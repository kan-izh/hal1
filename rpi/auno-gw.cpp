#include <cstdlib>
#include <iostream>
#include <time.h>

#include <RF24.h>
#include <stdio.h>
#include "../conf/rf-comm.h"
#include "../conf/messages.h"
#include "CommChannel.h"

typedef CommChannel<1024, RF_PAYLOAD_SIZE> RfCommChannel;

struct RpiTimeSource : TimeSource
{
	virtual uint32_t currentMicros()
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return ts.tv_nsec / 1000U + ts.tv_sec * 1000000U;
	}
};

RpiTimeSource timeSource;

void dump(const char *what, uint8_t const *buf, uint8_t len)
{
	return;
	printf("%u %s", timeSource.currentMicros(), what);
	for (int i = 0; i < len; ++i)
	{
		printf("%02X", int(buf[i]));
	}
	printf("\n");

}


struct RF24Output : CommChannelOutput
{

	RF24 &radio;

	RF24Output(RF24 &radio) : radio(radio)
	{
	}

	virtual void write(uint8_t const *buf, uint8_t len)
	{
		dump("Send: ", buf, len);
		radio.stopListening();
		radio.write(buf, len, true);
		radio.startListening();
	}
};

uint32_t convertTemperature(const uint16_t value)
{
	return (uint32_t(125) * value * 1000) / 256;
}

void display(uint32_t temperatureInMilliC);

int currentStrike = 0, bestStrike = 0;

struct RfReceiver : RfCommChannel::Receiver
{
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

	virtual void receive(Accessor &input)
	{
		currentStrike++;
		const uint8_t &contentId = input.template take<uint8_t>();
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

	virtual void restart()
	{
		bestStrike = std::max(bestStrike, currentStrike);
		currentStrike = 0;
		printf("restart\n");
		fflush(stdout);
	}
};

bool work = true;

void loop(RF24 radio, RfCommChannel &subscriber);

int main(int argc, char *argv[])
{
	RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);
	radio.begin();
	radio.setPayloadSize(RF_PAYLOAD_SIZE);
	radio.setAutoAck(false);
	radio.setChannel(RF_COMM_CHANNEL_ARDUINO_TO_RPI);
	radio.setPALevel(RF24_PA_MAX);
	radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_COMM_ADDRESS_RPI_TO_ARDUINO);
	radio.openReadingPipe(1, RF_COMM_ADDRESS_ARDUINO_TO_RPI);
	radio.startListening();
	radio.printDetails();
	fflush(stdout);

	RF24Output rf24Output(radio);
	RfReceiver receiver;
	RfCommChannel channel(timeSource, rf24Output, receiver);

	while(work)
	{
		loop(radio, channel);
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
	printf("[%d/%d] average(%d): %6.2f, current: %6.2f\r", currentStrike, bestStrike, cnt, avg / 1000., temperatureInMilliC / 1000.);
	fflush(stdout);
}


void loop(RF24 radio, RfCommChannel &channel)
{
	while (radio.available())
	{
		pulse = 0;
		radio.read(channel.getBuf(), channel.getBufSize());
		dump("Recv: ", channel.getBuf(), channel.getBufSize());
		channel.processBuf();
	}
	channel.processIdle();
	delay(10);
	if (++pulse % 200 == 0)
	{
		printf("\npulse %d\n", pulse);
		fflush(stdout);
	}
}
