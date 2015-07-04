#include <queue>
#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CommChannel.h"

const int64_t someData = -1951245124;

namespace
{
	using ::testing::_;

	static const int bufferSize = 7;
	static const int payloadSize = 23;
	typedef CommChannel<bufferSize, payloadSize> TestSubject;
	typedef TestSubject::BufferAccessor BufferAccessor;

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

		virtual void write(uint8_t const *buf, uint8_t len)
		{
			std::vector<uint8_t> payload;
			payload.assign(buf, buf + len);
			inFlight.push(payload);
		}

		void transferTo(TestSubject &target, int numberOfFrames = std::numeric_limits<int>::max())
		{
			while (numberOfFrames-- > 0 && !inFlight.empty())
			{
				std::vector<uint8_t> &payload = inFlight.front();
				ASSERT_LE(payload.size(), target.getBufSize());
				memcpy(target.getBuf(), &payload[0], payload.size() * sizeof(uint8_t));
				target.processBuf();
				inFlight.pop();
			}
		}
	};

	MATCHER_P(HasData, data, "")
	{
		data_type actual = arg.template take<data_type>();
		*result_listener << "Data is " << actual;
		return actual == data;
	}

	struct CommChannelTest : public ::testing::Test
	{
		MockTimeSource timeSource;
		TestSubject testSubject1;
		TestSubject testSubject2;
		MockCommChannelConnection connection;
		MockReceiver receiver1;
		MockReceiver receiver2;

		CommChannelTest()
				: testSubject1(timeSource, connection, receiver1)
				, testSubject2(timeSource, connection, receiver2)
		{ }

	};

	TEST_F(CommChannelTest, shouldDoSomething)
	{
		testSubject1.sendFrame(testSubject1.currentFrame()
				.put(someData)
		);
		EXPECT_CALL(receiver2, receive(HasData(someData)));
		connection.transferTo(testSubject2);
	}

}