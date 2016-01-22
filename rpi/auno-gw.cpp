#include <cstdlib>
#include <iostream>
#include <time.h>

#include <RF24.h>
#include <stdio.h>
#include "../conf/rf-comm.h"
#include "../conf/messages.h"
#include "CommChannel.h"

bool work = true;

int main(int argc, char *argv[])
{
	RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);
	radio.begin();
	radio.setRetries(3, 15);
	radio.setPayloadSize(RF_PAYLOAD_SIZE);
	radio.setAutoAck(true);
	radio.setChannel(RF_COMM_CHANNEL_ARDUINO_TO_RPI);
	radio.setPALevel(RF24_PA_MIN);
	radio.setDataRate(RF24_250KBPS);
	radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_COMM_ADDRESS_RPI_TO_ARDUINO);
	radio.openReadingPipe(1, RF_COMM_ADDRESS_ARDUINO_TO_RPI);
	radio.startListening();
	radio.printDetails();
	fflush(stdout);

	int64_t cnt=0;
	radio.stopListening();
	while(work)
	{
		//loop(radio, channel);
		cnt++;
		radio.write(&cnt, sizeof(cnt));
		delay(100);
	}
	return 0;
}
