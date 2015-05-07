#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "RingBufferPublisher.h"
#include "RingBufferSubscriber.h"
namespace
{
	using ::testing::_;

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
		MOCK_METHOD1(nak, void(const uint32_t &subscriberHighWatermark));
	};

	struct RingBufferLoopBackOutput : RingBufferOutput
	{
		bool broken;
		TestSubject *subscriber;
		RingBufferLoopBackOutput(TestSubject *subscriber)
				: subscriber(subscriber)
				, broken(false)
		{}

		virtual void write(uint8_t const *buf, uint8_t len)
		{
			if(broken)
			{
				return;
			}
			ASSERT_GE(subscriber->getBufSize(), len);
			memcpy(subscriber->getBuf(), buf, len);
			subscriber->process();
		}
	};

	struct RingBufferSubscriberTest : ::testing::Test
	{
		MockHandler handler;
		TestSubject testSubject;
		RingBufferLoopBackOutput loopBackOutput;
		Publisher publisher;

		RingBufferSubscriberTest()
				: testSubject(&handler)
				, loopBackOutput(&testSubject)
				, publisher(&loopBackOutput)
		{}

		void send()
		{
			publisher.getSendBuffer()
					.put(contentId)
					.put(data);
			publisher.send();
		}

		void sendAndExpectFirst()
		{
			EXPECT_CALL(handler, handle(1U, contentId, HasData(data)))
					.Times(1);
			send();
		}

	};

	TEST_F(RingBufferSubscriberTest, shouldSendData)
	{
		sendAndExpectFirst();
	}

	TEST_F(RingBufferSubscriberTest, shouldLateJoin)
	{
		const uint32_t initialHwm = 84;

		EXPECT_CALL(handler, handle(initialHwm, contentId, HasData(data)))
				.Times(1);

		publisher.setHighWatermark(initialHwm);
		send();
	}

	TEST_F(RingBufferSubscriberTest, shouldNak)
	{
		sendAndExpectFirst();
		loopBackOutput.broken = true;
		send();

		loopBackOutput.broken = false;

		EXPECT_CALL(handler, handleNak(2U))
				.Times(1);
		send();
	}

	TEST_F(RingBufferSubscriberTest, shouldRecoverAfterNaked)
	{
		sendAndExpectFirst();
		loopBackOutput.broken = true;
		send();
		loopBackOutput.broken = false;

		EXPECT_CALL(handler, handleNak(2U))
				.Times(1);
		send();

		EXPECT_CALL(handler, handle(2U, contentId, HasData(data)))
				.Times(1);
		EXPECT_CALL(handler, handle(3U, contentId, HasData(data)))
				.Times(1);
		publisher.nak(2U);
	}

	TEST_F(RingBufferSubscriberTest, shouldIgnoreReceived)
	{
		sendAndExpectFirst();
		EXPECT_CALL(handler, handleNak(_))
				.Times(0);
		publisher.nak(1U);
	}
}
