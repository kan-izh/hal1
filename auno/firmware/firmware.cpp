#include "Arduino.h"
#include <RF24.h>
#include <IRremote.h>
#include "HardwareSerial.h"
#include "rf-settings.h"
#include "../../conf/messages.h"
#include "CommChannel.h"

//after build -- to flash
//avrdude -V -c arduino -p m328p -b 115200 -P /dev/ttyACM3 -U flash:w:.build/firmware/firmware.hex
static const int bufferSize = 64;
typedef CommChannel<bufferSize, RF_PAYLOAD_SIZE> ArduinoCommChannel;

struct ArduinoTimeSource : TimeSource
{
	virtual uint32_t currentMicros()
	{
		return static_cast<uint32_t>(micros());
	}
};

ArduinoTimeSource timeSource;
bool dumpEnabled = false;

void dump(const char *what, uint8_t const *buf, uint8_t len)
{
	if(!dumpEnabled)
		return;
	Serial.print(timeSource.currentMicros());
	Serial.print(" ");
	Serial.print(what);
	for (int i = 0; i < len; ++i)
	{
		Serial.print(buf[i] >> 4, 16);
		Serial.print(buf[i] & 0x0f, 16);
	}
	Serial.println();

}

struct ArduinoReceiver : ArduinoCommChannel::Receiver
{
	virtual void receive(Accessor &accessor)
	{
	}

	virtual void restart()
	{
		Serial.println("restart channel");
	}
};

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

const unsigned long heartbeatIntervalMs = 25;
unsigned long timeMs = 0;

void schedule(RF24 &radio, ArduinoCommChannel &publisher);
void listenRf(RF24 &radio, ArduinoCommChannel &subscriber);

void sendAnalogRead(ArduinoCommChannel &channel, const uint8_t i);

bool work = true;

IRrecv irRecv(6);

decode_results results;

void irRemote(ArduinoCommChannel &channel);

int main()
{
	init();
    Serial.begin(115200);
	pinMode(7, INPUT);
	irRecv.enableIRIn();

	RF24 radio(RF_cepin, RF_cspin);
	radio.begin();
	radio.setPayloadSize(RF_PAYLOAD_SIZE);
	radio.setAutoAck(false);
    radio.setChannel(RF_channel);
	radio.setPALevel(RF24_PA_MAX);
    radio.setAddressWidth(RF_COMM_ADDRESS_WIDTH);
	radio.openWritingPipe(RF_address_write);
	radio.openReadingPipe(1, RF_address_read);
	radio.startListening();

    RF24Output rf24Output(radio);
	ArduinoReceiver receiver;
	ArduinoCommChannel channel(timeSource, rf24Output, receiver);
    Serial.println("initialised");
    Serial.flush();
    while(work)
	{
		listenRf(radio, channel);
		channel.processIdle();
		schedule(radio, channel);
		irRemote(channel);
		dumpEnabled = digitalRead(7) == LOW;
    }
	return 0;
}

void listenRf(RF24 &radio, ArduinoCommChannel &channel)
{
	while(radio.available())
	{
		radio.read(channel.getBuf(), channel.getBufSize());
		dump("Recv: ", channel.getBuf(), channel.getBufSize());
		channel.processBuf();
	}
}

void schedule(RF24 &radio, ArduinoCommChannel &channel)
{
    unsigned long currentTime = millis();
    if(currentTime - timeMs > heartbeatIntervalMs)
    {
        timeMs = currentTime;
		sendAnalogRead(channel, TEMPERATURE_SENSOR);
    }
}

void sendAnalogRead(ArduinoCommChannel &channel, const uint8_t pin)
{
	const uint16_t value = (uint16_t) analogRead(pin);
	channel.sendFrame(channel.currentFrame()
				.put(MESSAGE_ANALOG_READ)
				.put(pin)
				.put(value));
}

void irRemote(ArduinoCommChannel &channel)
{
	if(irRecv.decode(&results))
	{
		uint32_t value = (uint32_t) results.value;
		irRecv.resume();

		channel.sendFrame(channel.currentFrame()
				.put(MESSAGE_IR_COMMAND)
				.put(value));
	}
}