#include "gtest/gtest.h"
#include "RingBufferPublisher.h"

static const int bufferSize = 10;
typedef RingBufferPublisher<bufferSize, 32> TestSubject;

struct MockRingBufferOutput : RingBufferOutput
{
	std::vector< TestSubject::PayloadBuffer > written;
	virtual void write(uint8_t const *buf, uint8_t len)
	{
		written.push_back(TestSubject::PayloadBuffer());
		memcpy(written.back().getBuf(), buf, len);
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
	const uint32_t requestedHwm = 2;
	testSubject.nak(requestedHwm);
	ASSERT_EQ(1U, output.written.size());
	TestSubject::PayloadAccessor actual(&output.written[0]);
	const uint32_t &actualHwm = actual.template take<uint32_t>();
	const uint32_t &actualRequestedHwm = actual.template take<uint32_t>();
	ASSERT_EQ(CONTROL_BAD_NAK_ID, actualHwm);
	ASSERT_EQ(requestedHwm, actualRequestedHwm);
}

TEST(RingBufferPublisherTest, outOfRangeNak)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	testSubject.setHighWatermark(1000);
	const uint32_t requestedHwm = 2;
	testSubject.nak(requestedHwm);
	ASSERT_EQ(1U, output.written.size());
	TestSubject::PayloadAccessor actual(&output.written[0]);
	const uint32_t &actualHwm = actual.template take<uint32_t>();
	const uint32_t &actualRequestedHwm = actual.template take<uint32_t>();
	ASSERT_EQ(CONTROL_NAK_BUFFER_OVERFLOW, actualHwm);
	ASSERT_EQ(requestedHwm, actualRequestedHwm);
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
	ASSERT_EQ(33U, TestSubject::PayloadAccessor(&output.written[0]).template take<uint32_t>());
	ASSERT_EQ(34U, TestSubject::PayloadAccessor(&output.written[1]).template take<uint32_t>());
	ASSERT_EQ(35U, TestSubject::PayloadAccessor(&output.written[2]).template take<uint32_t>());
	// nak lost messages
	output.written.clear();
	testSubject.nak(34);
	ASSERT_EQ(36U, testSubject.getHighWatermark());
	ASSERT_EQ(2U, output.written.size());
	ASSERT_EQ(34U, TestSubject::PayloadAccessor(&output.written[0]).template take<uint32_t>());
	ASSERT_EQ(35U, TestSubject::PayloadAccessor(&output.written[1]).template take<uint32_t>());
}

TEST(RingBufferPublisherTest, sendData)
{
	MockRingBufferOutput output;
	const uint16_t data1 = 42;
	const int64_t data2 = -24;
	TestSubject testSubject(&output);
	testSubject.getSendBuffer()
			.put(data1)
			.put(data2);
	testSubject.send();
	ASSERT_EQ(2U, testSubject.getHighWatermark());
	ASSERT_EQ(1U, output.written.size());
	TestSubject::PayloadAccessor actual(&output.written[0]);
	const uint32_t &actualHwm = actual.template take<uint32_t>();
	const uint16_t &actualData1 = actual.template take<uint16_t>();
	const int64_t &actualData2 = actual.template take<int64_t>();
	ASSERT_EQ(1U, actualHwm);
	ASSERT_EQ(data1, actualData1);
	ASSERT_EQ(data2, actualData2);
}

TEST(RingBufferPublisherTest, heartbeat)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	testSubject.heartbeat();
	ASSERT_EQ(1U, testSubject.getHighWatermark());
	ASSERT_EQ(1U, output.written.size());
	TestSubject::PayloadAccessor actual(&output.written[0]);
	const uint32_t &actualHwm = actual.template take<uint32_t>();
	const uint32_t &msgId = actual.template take<uint8_t >();
	ASSERT_EQ(1U, actualHwm);
	ASSERT_EQ(0, msgId);
}

TEST(RingBufferPublisherTest, bufferWrap)
{
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	for (int i = 1; i <= bufferSize * 2; ++i)
	{
		testSubject.getSendBuffer().put(i * 2);
		testSubject.send();
	}
	ASSERT_EQ(bufferSize * 2, output.written.size());
	for (int i = 1; i <= bufferSize * 2; ++i)
	{
		TestSubject::PayloadAccessor actual(&output.written[i - 1]);
		const uint32_t &actualHwm = actual.template take<uint32_t>();
		const int &actualData = actual.template take<int>();
		ASSERT_EQ(i, actualHwm);
		ASSERT_EQ(i * 2, actualData);
	}
}

TEST(RingBufferPublisherTest, overflowHwm)
{
	const int32_t data = -1151414542;
	MockRingBufferOutput output;
	TestSubject testSubject(&output);
	const uint32_t lastAvailableHwm = MAX_HIGH_WATERMARK_VALUE - 1;
	testSubject.setHighWatermark(lastAvailableHwm);
	testSubject.getSendBuffer()
			.put(data);
	testSubject.send();
	testSubject.getSendBuffer()
			.put(data + 1);
	testSubject.send();
	ASSERT_EQ(2U, output.written.size());
	TestSubject::PayloadAccessor actual1(&output.written[0]);
	const uint32_t &actualHwm1 = actual1.template take<uint32_t>();
	const int32_t &actualData1 = actual1.template take<int32_t>();
	ASSERT_EQ(lastAvailableHwm, actualHwm1);
	ASSERT_EQ(data, actualData1);
	TestSubject::PayloadAccessor actual2(&output.written[1]);
	const uint32_t &actualHwm2 = actual2.template take<uint32_t>();
	ASSERT_EQ(CONTROL_MAX_HIGH_WATERMARK_ID, actualHwm2);
}
