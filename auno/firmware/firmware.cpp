#include "Arduino.h"
#include <RF24.h>
#include <IRremote.h>
#include "HardwareSerial.h"
#include "rf-settings.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/firmware/firmware.hex

void listenRf(RF24 &radio);

bool work = true;

IRrecv irRecv(6);

decode_results results;

int main()
{
	init();
    Serial.begin(115200);
	irRecv.enableIRIn();

	RF24 radio(RF_cepin, RF_cspin);
	radio.begin();
	radio.setPayloadSize(RF_PAYLOAD_SIZE);
	radio.setAutoAck(true);
    radio.setChannel(RF_channel);
	radio.setPALevel(RF24_PA_MIN);
	radio.setDataRate(RF24_250KBPS);
    radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_address_write);
	radio.openReadingPipe(1, RF_address_read);
	radio.startListening();

    Serial.println("initialised");
    Serial.flush();
    while(work)
	{
		listenRf(radio);
    }
	return 0;
}

uint64_t curVal = 0;
uint32_t totalLoss = 0;
uint8_t buf[RF_PAYLOAD_SIZE];
void listenRf(RF24 &radio)
{
	while(radio.available())
	{
		radio.read(buf, RF_PAYLOAD_SIZE);
		uint64_t *val = (uint64_t *)buf;
		uint32_t lost = *val - curVal - 1;
		curVal = *val;
		if(lost != 0)
		{
			totalLoss += lost;
			Serial.print("lost ");
			Serial.print(lost);
			Serial.print(", total ");
			Serial.println(totalLoss);
		}
	}
	if(irRecv.decode(&results))
	{
		uint32_t value = (uint32_t) results.value;
		irRecv.resume();

		Serial.print(micros());
		Serial.print(" ");
		Serial.print("IR: ");
		Serial.println(value, HEX);
	}
}
