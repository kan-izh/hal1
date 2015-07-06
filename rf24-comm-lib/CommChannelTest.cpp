#include <queue>
#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CommChannel.h"

const int64_t someData1 = -1918171615;
const uint32_t someData2 = 2021222324;

namespace
{
	using namespace ::testing;

	static const int bufferSize = 7;
	static const int payloadSize = 23;
	typedef CommChannel<bufferSize, payloadSize> TestSubject;

	struct MockTimeSource : TimeSource
	{
		MOCK_METHOD0(currentMicros, uint32_t());
	};

	struct MockReceiver : TestSubject::Receiver
	{
		MOCK_METHOD1(receive, void(typename ByteBuffer<payloadSize>::Accessor &));
	};


	struct MockCommChannelConnection : CommChannelOutput
	{
		std::queue<std::vector<uint8_t> > inFlight;
		TestSubject *target;

		virtual void write(uint8_t const *buf, uint8_t len)
		{
			std::vector<uint8_t> payload;
			payload.assign(buf, buf + len);
			inFlight.push(payload);
		}

		void drop(int numberOfFrames)
		{
			while (numberOfFrames-- > 0)
			{
				inFlight.pop();
			}
		}

		void transfer(int numberOfFrames)
		{
			ASSERT_LE(numberOfFrames, inFlight.size());
			while (numberOfFrames-- > 0 && !inFlight.empty())
			{
				std::vector<uint8_t> &payload = inFlight.front();
				ASSERT_LE(payload.size(), target->getBufSize());
				memcpy(target->getBuf(), &payload[0], payload.size() * sizeof(uint8_t));
				target->processBuf();
				inFlight.pop();
			}
		}

		void assertNoFrames()
		{
			ASSERT_EQ(0, inFlight.size());
		}
	};

	MATCHER_P(HasData, expected, "")
	{
		expected_type actual = arg.template take<expected_type>();
		*result_listener << "actual = " << actual << ", expected = " << expected;
		return actual == expected;
	}

	static const uint32_t BASE_TIME = 123000;

	struct CommChannelTest : public ::testing::Test
	{
		MockTimeSource timeSource;
		TestSubject testSubject1;
		TestSubject testSubject2;
		MockCommChannelConnection connection12;
		MockCommChannelConnection connection21;
		MockReceiver receiver1;
		MockReceiver receiver2;

		CommChannelTest()
				: testSubject1(timeSource, connection12, receiver1)
				, testSubject2(timeSource, connection21, receiver2)
		{
			EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME));
			connection12.target = &testSubject2;
			connection21.target = &testSubject1;
		}

	};

	TEST_F(CommChannelTest, shouldSendSimpleOneFrame)
	{
		EXPECT_CALL(receiver2, receive(HasData(someData1)));

		testSubject1.sendFrame(testSubject1.currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);
	}

	TEST_F(CommChannelTest, shouldSendSimpleTwoFrames)
	{
		::testing::Sequence receiverSeq;
		EXPECT_CALL(receiver2, receive(HasData(someData1)))
				.InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData2)))
				.InSequence(receiverSeq);

		testSubject1.sendFrame(testSubject1.currentFrame()
				.put(someData1)
		);
		testSubject1.sendFrame(testSubject1.currentFrame()
				.put(someData2)
		);
		connection12.transfer(2);
	}

	TEST_F(CommChannelTest, shouldRecoverOneFrame)
	{
		EXPECT_CALL(receiver2, receive(HasData(someData1)));

		testSubject1.sendFrame(testSubject1.currentFrame()
				.put(someData1)
		);
		connection12.drop(1);
		testSubject1.processIdle();
		connection12.assertNoFrames();

		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1.getTimeoutMicros() - 1));
		testSubject1.processIdle();
		connection12.assertNoFrames();

		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1.getTimeoutMicros()));
		testSubject1.processIdle();
		connection12.transfer(1);
	}

	TEST_F(CommChannelTest, shouldAckOneFrame)
	{
		EXPECT_CALL(receiver2, receive(HasData(someData1)));

		testSubject1.sendFrame(testSubject1.currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);
		testSubject2.processIdle();
		connection21.transfer(1);

		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1.getTimeoutMicros()));
		testSubject1.processIdle();
		connection12.assertNoFrames();
	}
}
