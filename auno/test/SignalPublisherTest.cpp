#include "gtest/gtest.h"
#include "SignalPublisher.h"
#include "MockStream.h"

int sos = 0;
extern "C" void showSos()
{
	++sos;
}

int wrongPin = 0;
int pinValue[16] = {-1};
extern "C" int analogRead(uint8_t pin)
{
	int value = pinValue[pin];
	if(value == -1)
		++wrongPin;
	return value;
}

TEST(SignalPublisherTest, send)
{
	MockStream mockStream;
	SignalPublisher publisher(&mockStream);
	pinValue[0] = 42;
	publisher.send();

	pinValue[0] = 84;
	publisher.send();

	ASSERT_NE((size_t)0, mockStream.written.size());

	ASSERT_EQ(0, sos);
	ASSERT_EQ(0, wrongPin);
}
