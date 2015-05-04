#include <string>
#include <ios>
#include "gtest/gtest.h"
#include "RingBufferPublisher.h"
#include "RingBufferSubscriber.h"
namespace
{
	static const int bufferSize = 10;
	static const uint8_t contentId = 73;
	static const uint64_t data = 9136457987143567615ULL;
	typedef RingBufferPublisher<bufferSize, 32> Publisher;
	typedef RingBufferSubscriber<32> TestSubject;

	struct MockHandler : TestSubject::Handler
	{
		std::stringstream result;

		virtual void handle(uint32_t messageId, uint8_t contentId, RingBufferSubscriber<32>::PayloadAccessor &input)
		{
			result << "handle("
			<< messageId << ", "
			<< int(contentId) << ", "
			<< input.template take<uint64_t>()
			<< ")" << std::endl;
		}

		virtual void handleNak(uint32_t hwm)
		{
		}
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
		publisher.getSendBuffer()
				.put(contentId)
				.put(data);
		publisher.send();
		ASSERT_EQ("handle(1, 73, 9136457987143567615)\n", handler.result.str());
	}
}
