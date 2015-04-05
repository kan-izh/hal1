#include <cstdlib>
#include <iostream>

#include <RF24.h>
#include "../conf/rf-comm.h"

bool work = true;

void loop(RF24 &radio);

int main(int argc, char *argv[])
{
	RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);
	radio.begin();
	radio.setChannel(RF_COMM_CHANNEL_ARDUINO_TO_RPI);
	radio.setPALevel(RF24_PA_MAX);
	radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_COMM_ADDRESS_RPI_TO_ARDUINO);
	radio.openReadingPipe(1, RF_COMM_ADDRESS_ARDUINO_TO_RPI);
	radio.startListening();
	radio.printDetails();
	fflush(stdout);
	while(work)
	{
		loop(radio);
	}
	return 0;
}

const int tempHistorySize = 1024;
int tempHistory[tempHistorySize] = {};
int tempHistoryPos = 0;
int pulse=0;

void loop(RF24 &radio)
{
	uint8_t buf[32];
	if (radio.available())
	{
		pulse = 0;
		radio.read(buf, 32);
		int hwm = *(int *) (buf);
		int temp = *(int *) (buf + 4);
		tempHistory[tempHistoryPos] = temp;
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
		printf("%d: average(%d): %6.2f, current: %6.2f\n", hwm, cnt, avg / 100., temp / 100.);
		fflush(stdout);
	}
	delay(100);
	if (++pulse % 20 == 0)
	{
		printf("pulse %d\n", pulse);
		fflush(stdout);
	}
}
