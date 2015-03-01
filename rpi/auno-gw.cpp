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
	radio.setChannel(0x4c);
	radio.openReadingPipe(1, RF_COMM_ADDRESS_ARDUINO_TO_RPI);
	radio.openWritingPipe(RF_COMM_ADDRESS_RPI_TO_ARDUINO);
	radio.startListening();
	radio.printDetails();
	while(work)
	{
		loop(radio);
	}
	return 0;
}

const int tempHistorySize = 1024;
int tempHistory[tempHistorySize] = {};
int tempHistoryPos = 0;

void loop(RF24 &radio)
{
	uint8_t buf[32];
	if (radio.available())
	{
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
}
