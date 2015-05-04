#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "RingBufferPublisher.h"
#include "RingBufferSubscriber.h"
namespace
{
	static const int bufferSize = 10;
	static const uint8_t contentId = 73;
	static const uint64_t data = 9136457987143567615ULL;
	typedef RingBufferPublisher<bufferSize, 32> Publisher;
	typedef RingBufferSubscriber<32> TestSubject;

	MATCHER_P(HasData, data, "") 
	{
		data_type actual = arg.template take<uint64_t>();
		*result_listener << "Data is " << actual;
		return actual == data;
	}

	struct MockHandler : public TestSubject::Handler
	{
		MOCK_METHOD3(handle, void(uint32_t messageId, uint8_t contentId, TestSubject::PayloadAccessor &input));
		MOCK_METHOD1(handleNak, void(uint32_t hwm));
	};

	struct RingBufferLoopBackOutput : RingBufferOutput
	{
		TestSubject *subscriber;
		RingBufferLoopBackOutput(TestSubject *subscriber)
				: subscriber(subscriber)
		{}

		virtual void write(uint8_t const *buf, uint8_t len)
		{
			ASSERT_GE(subscriber->getBufSize(), len);
			memcpy(subscriber->getBuf(), buf, len);
			subscriber->process();
		}
	};

	TEST(RingBufferSubscriberTest, shouldSendData)
	{
		MockHandler handler;
		TestSubject testSubject(&handler);
		RingBufferLoopBackOutput loopBackOutput(&testSubject);
		Publisher publisher(&loopBackOutput);

		EXPECT_CALL(handler, handle(1U, contentId, HasData(data)))
				.Times(1);

		publisher.getSendBuffer()
				.put(contentId)
				.put(data);
		publisher.send();
	}
}
