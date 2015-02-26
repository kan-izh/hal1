#include "gtest/gtest.h"
#include "RingBufferPublisher.h"

typedef RingBufferPublisher<128, 32> TestSubject;

struct MockRingBufferOutput : RingBufferOutput
{
	std::vector< std::vector<uint8_t> > written;
	virtual void write(uint8_t const *buf, uint8_t len)
	{
		std::vector<uint8_t> value;
		value.assign(buf, buf+len);
		written.push_back(value);
	}
};

static uint32_t hwmCast(const uint8_t *buf)
{
	return *reinterpret_cast<const uint32_t *>(buf);
}

TEST(RingBufferPublisherTest, badNak)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	ASSERT_EQ(1U, testSubject.getHighWatermark());
	testSubject.nak(2);
	ASSERT_EQ(1U, output.written.size());
	const std::vector<uint8_t> &actual = output.written[0];
	ASSERT_EQ(32U, actual.size());
	ASSERT_EQ(CONTROL_BAD_NAK_ID, hwmCast(actual.data()));
}

TEST(RingBufferPublisherTest, outOfRangeNak)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	testSubject.setHighWatermark(1000);
	testSubject.nak(2);
	ASSERT_EQ(1U, output.written.size());
	const std::vector<uint8_t> &actual = output.written[0];
	ASSERT_EQ(32U, actual.size());
	ASSERT_EQ(CONTROL_NAK_BUFFER_OVERFLOW, hwmCast(actual.data()));
}

TEST(RingBufferPublisherTest, nakCorrectly)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	testSubject.setHighWatermark(33);
	testSubject.send();
	testSubject.send();
	testSubject.send();
	ASSERT_EQ(36U, testSubject.getHighWatermark());
	ASSERT_EQ(3U, output.written.size());
	ASSERT_EQ(33U, hwmCast(output.written[0].data()));
	ASSERT_EQ(34U, hwmCast(output.written[1].data()));
	ASSERT_EQ(35U, hwmCast(output.written[2].data()));
	// nak lost messages
	output.written.clear();
	testSubject.nak(34);
	ASSERT_EQ(36U, testSubject.getHighWatermark());
	ASSERT_EQ(2U, output.written.size());
	ASSERT_EQ(34U, hwmCast(output.written[0].data()));
	ASSERT_EQ(35U, hwmCast(output.written[1].data()));
}

TEST(RingBufferPublisherTest, sendData)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	memset(testSubject.getSendBuffer(), 42, testSubject.getSendBufferSize());
	testSubject.send();
	ASSERT_EQ(2U, testSubject.getHighWatermark());
	ASSERT_EQ(1U, output.written.size());
	const std::vector<uint8_t> &actual = output.written[0];
	ASSERT_EQ(32U, actual.size());
	ASSERT_EQ(1U, hwmCast(actual.data()));
	ASSERT_EQ(42, actual[5]);
}

TEST(RingBufferPublisherTest, heartbeat)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	testSubject.heartbeat();
	ASSERT_EQ(1U, testSubject.getHighWatermark());
	ASSERT_EQ(1U, output.written.size());
	const std::vector<uint8_t> &actual = output.written[0];
	ASSERT_EQ(32U, actual.size());
	ASSERT_EQ(1U, hwmCast(actual.data()));
	ASSERT_EQ(0, actual[5]);
}
