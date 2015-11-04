#include <vector>
#include <deque>
#include <backward/auto_ptr.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CommChannel.h"

const int64_t someData1 = int64_t(0x1918171615141312);
const uint32_t someData2 = uint32_t(0x20212223);
const uint8_t someData3 = 0x33;

std::ostream &operator<<(std::ostream &os, const std::deque<std::vector<uint8_t> > &inFlight)
{
	for(std::deque<std::vector<uint8_t> >::const_iterator payloadIt = inFlight.begin();
			payloadIt != inFlight.end();
			++payloadIt)
	{
		for(std::vector<uint8_t>::const_iterator it = payloadIt->begin(); it != payloadIt->end(); ++it)
		{
			if(it != payloadIt->begin())  os << ":";
			os << std::setfill('0') << std::setw(2) << std::hex << int(*it);
		}
		os << std::endl;
	}
	return os;
}

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
		MOCK_METHOD0(restart, void());
	};

	struct MockCommChannelConnection : CommChannelOutput
	{
		std::deque<std::vector<uint8_t> > inFlight;
		TestSubject *target;

		~MockCommChannelConnection()
		{
			assertNoFrames();
		}

		virtual void write(uint8_t const *buf, uint8_t len)
		{
			std::vector<uint8_t> payload;
			payload.assign(buf, buf + len);
			inFlight.push_back(payload);
		}

		void drop(int numberOfFrames)
		{
			while (numberOfFrames-- > 0)
			{
				inFlight.pop_front();
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
				inFlight.pop_front();
			}
		}

		void assertNoFrames()
		{
			ASSERT_EQ(0, inFlight.size()) << inFlight;
		}
	};

	MATCHER_P(HasData, expected, "")
	{
		expected_type actual = arg.template take<expected_type>();
		*result_listener << std::hex << "actual = " << actual << ", expected = " << expected;
		return actual == expected;
	}

	static const uint32_t BASE_TIME = 123000;

	struct CommChannelTest : public ::testing::Test
	{
		MockTimeSource timeSource;
		std::auto_ptr<TestSubject> testSubject1;
		std::auto_ptr<TestSubject> testSubject2;
		MockCommChannelConnection connection12;
		MockCommChannelConnection connection21;
		MockReceiver receiver1;
		MockReceiver receiver2;

		CommChannelTest()
				: testSubject1(new TestSubject(timeSource, connection12, receiver1))
				, testSubject2(new TestSubject(timeSource, connection21, receiver2))
		{
			EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME));
			connection12.target = testSubject2.get();
			connection21.target = testSubject1.get();
		}

		void oneInitFrame(const Sequence &seq)
		{
			EXPECT_CALL(receiver2, restart()).InSequence(seq);
			EXPECT_CALL(receiver2, receive(HasData(someData1))).InSequence(seq);
			testSubject1->sendFrame(testSubject1->currentFrame()
					.put(someData1)
			);
			connection12.transfer(1);
			testSubject2->processIdle();
			connection21.transfer(1);
		}

		void restartTestSubject1()
		{
			testSubject1.reset(new TestSubject(timeSource, connection12, receiver1));
			connection21.target = testSubject1.get();
		}
	};

	TEST_F(CommChannelTest, shouldSendSimpleOneFrame)
	{
		Sequence seq;
		oneInitFrame(seq);
	}

	TEST_F(CommChannelTest, shouldSendSimpleTwoFrames)
	{
		Sequence receiverSeq;
		EXPECT_CALL(receiver2, restart()).InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData1))).InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData2))).InSequence(receiverSeq);

		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);//send
		testSubject2->processIdle();
		connection21.transfer(1);//acked
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData2)
		);
		connection12.transfer(1);//send
		testSubject2->processIdle();
		connection21.transfer(1);//acked
	}

	TEST_F(CommChannelTest, shouldRecoverOneFrame)
	{
		EXPECT_CALL(receiver2, restart());
		EXPECT_CALL(receiver2, receive(HasData(someData1)));

		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.drop(1);
		testSubject1->processIdle();
		connection12.assertNoFrames();

		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1->getTimeoutMicros() - 1));
		testSubject1->processIdle();
		connection12.assertNoFrames();

		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1->getTimeoutMicros()));
		testSubject1->processIdle();
		connection12.transfer(1);//re-send
		testSubject2->processIdle();
		connection21.transfer(1);//acked
	}

	TEST_F(CommChannelTest, shouldAckOneFrame)
	{
		EXPECT_CALL(receiver2, restart());
		EXPECT_CALL(receiver2, receive(HasData(someData1)));

		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);
		testSubject2->processIdle();//consume
		connection21.transfer(1);//acked

		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1->getTimeoutMicros()));
		testSubject1->processIdle();//not re-send
	}

	TEST_F(CommChannelTest, shouldRecoverLostFrame)
	{
		Sequence receiverSeq;
		EXPECT_CALL(receiver2, restart()).InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData1))).InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData2))).InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData3))).InSequence(receiverSeq);

		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);//send 1
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData2)
		);
		testSubject2->processIdle();//consume 1
		connection21.transfer(1);//ack 1
		connection12.drop(1);//drop 2
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData3)
		);
		connection12.transfer(1);//send 3

		testSubject2->processIdle();//nak 2
		connection21.transfer(1);//nak 2
		connection12.transfer(1);//re-send 2
		testSubject2->processIdle();//consume 2, and cached 3, ack 3
		connection21.transfer(1);//send ack3
		connection12.transfer(1);//re-send duplicate 3
	}

	TEST_F(CommChannelTest, shouldLateJoin)
	{
		Sequence receiverSeq;
		EXPECT_CALL(receiver2, restart()).InSequence(receiverSeq);
		EXPECT_CALL(receiver2, receive(HasData(someData2))).InSequence(receiverSeq);

		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.drop(1);
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData2)
		);
		connection12.transfer(1);
		testSubject2->processIdle();
		connection21.transfer(1);
	}

	TEST_F(CommChannelTest, shouldSendWrappingBuffer)
	{
		Sequence receiverSeq;
		EXPECT_CALL(receiver2, restart()).InSequence(receiverSeq);
		for(int i=0; i<bufferSize*3; ++i)
		{
			const int expected = i * 3 + 7;
			EXPECT_CALL(receiver2, receive(HasData(expected))).InSequence(receiverSeq);
			testSubject1->sendFrame(testSubject1->currentFrame()
					.put(expected)
			);
			connection12.transfer(1);
			testSubject2->processIdle();
			connection21.transfer(1);
		}
	}

	TEST_F(CommChannelTest, shouldNotOverflowBuffer)
	{
		Sequence seq;
		oneInitFrame(seq);
		// drop frames less than buffer
		for(int i=0; i<bufferSize - 1; ++i)
		{
			const int expected = i * 3 + 7;
			EXPECT_CALL(receiver2, receive(HasData(expected))).InSequence(seq);
			testSubject1->sendFrame(testSubject1->currentFrame()
					.put(expected)
			);
			connection12.drop(1);
		}
		// one more data frame which goes through
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		EXPECT_CALL(receiver2, receive(HasData(someData1))).InSequence(seq);
		connection12.transfer(1);//data
		testSubject2->processIdle();
		connection21.transfer(1);//nak
		connection12.transfer(bufferSize);//all naked
		testSubject2->processIdle();
		connection21.transfer(1);//all acked
	}

	TEST_F(CommChannelTest, shouldDetectNakOverflow)
	{
		Sequence seq;
		oneInitFrame(seq);
		// drop frames enough to overflow buffer
		for(int i=0; i<bufferSize; ++i)
		{
			const int data = i * 3 + 7;
			testSubject1->sendFrame(testSubject1->currentFrame()
					.put(data)
			);
			connection12.drop(1);
		}
		// one more data frame
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);//data
		testSubject2->processIdle();
		connection21.transfer(1);//nak
		connection12.transfer(1);//nak overflow
		//following data is expected to restart the channel
		oneInitFrame(seq);
	}

	TEST_F(CommChannelTest, shouldHandleChannelRestart)
	{
		Sequence seq;
		oneInitFrame(seq);
		restartTestSubject1();
		oneInitFrame(seq);
	}

	TEST_F(CommChannelTest, shouldSkipWrongAck)
	{
		Sequence seq;
		oneInitFrame(seq);
		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData2)
		);
		EXPECT_CALL(receiver2, receive(HasData(someData2))).InSequence(seq);
		connection12.transfer(1);
		testSubject2->processIdle();//sending an ack
		restartTestSubject1();
		connection21.transfer(1);//receiving the ack, but too late, it's re-spawned sender now
		oneInitFrame(seq);//and should work as usual
	}

	TEST_F(CommChannelTest, shouldSupportDoubleFirstData)
	{
		Sequence seq;
		EXPECT_CALL(receiver2, restart()).InSequence(seq);//frame
		EXPECT_CALL(receiver2, restart()).InSequence(seq);//frame dup
		EXPECT_CALL(receiver2, receive(HasData(someData1)));

		testSubject1->sendFrame(testSubject1->currentFrame()
				.put(someData1)
		);
		connection12.transfer(1);//send
		EXPECT_CALL(timeSource, currentMicros()).WillRepeatedly(Return(BASE_TIME + testSubject1->getTimeoutMicros()));
		testSubject1->processIdle();
		connection12.transfer(1);//re-send as dup
		testSubject2->processIdle();
		connection21.transfer(1);//acked
	}
}
