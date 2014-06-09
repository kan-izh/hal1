#include "gtest/gtest.h"
#include "ScobsStream.h"
#include "MockStream.h"

#define SIZE_OF_ARRAY(arr) sizeof(arr) / sizeof(*arr)

class ScobsStreamTest : public ::testing::Test
{
public:
	MockStream mockStream;
	ScobsStream targetObject;

	ScobsStreamTest()
		: targetObject(&mockStream)
	{
	}

	const std::vector<uint8_t> &written()
	{
		targetObject.flush();
		return mockStream.written;
	}

	std::vector<uint8_t> read()
	{
		std::vector<uint8_t> result;
		for(int b; (b = targetObject.read()) != -1; )
		{
			result.push_back(b);
		}
		return result;
	}

	std::vector<uint8_t> newVector(const uint8_t *data, size_t len)
	{
		std::vector<uint8_t> expected;
		expected.assign(data, data + len);
		return expected;
	}

	void setInput(const uint8_t *data, size_t len)
	{
		for(size_t i = 0; i < len; ++i)
			mockStream.input.push(data[i]);
	}
};

TEST_F(ScobsStreamTest, write0)
{
	uint8_t data[] = {0x00};
	targetObject.write(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {-0x01};
	ASSERT_EQ(written(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, write1)
{
	uint8_t data[] = {0x11, 0x22, 0x00, 0x33};
	targetObject.write(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {-0x03, 0x11, 0x22, 0x01, 0x33};
	ASSERT_EQ(written(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, write2)
{
	uint8_t data[] = {0x11, 0x00, 0x00, 0x00};
	targetObject.write(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {-0x02, 0x11, -0x01, -0x01};
	ASSERT_EQ(written(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, write3)
{
	for(int i = 1; i <= 255; ++i)
		targetObject.write(i);
	const std::vector<uint8_t> &actual = written();
	ASSERT_EQ((size_t)258, actual.size());
	ASSERT_EQ(125, actual[0]);
	ASSERT_EQ(1, actual[1]);
	ASSERT_EQ(2, actual[2]);
	ASSERT_EQ(124, actual[124]);
	ASSERT_EQ(125, actual[125]);
	ASSERT_EQ(125, actual[126]);
	ASSERT_EQ(126, actual[127]);
	ASSERT_EQ(250, actual[251]);
	ASSERT_EQ(5, actual[252]);
	ASSERT_EQ(251, actual[253]);
	ASSERT_EQ(252, actual[254]);
	ASSERT_EQ(253, actual[255]);
	ASSERT_EQ(255, actual[257]);
}

TEST_F(ScobsStreamTest, write4)
{
	uint8_t data[] = {0x01};
	targetObject.write(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {0x01, 0x01};
	ASSERT_EQ(written(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, write5)
{
	uint8_t data[] = {0x00, 0x00};
	targetObject.write(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {-0x01, -0x01};
	ASSERT_EQ(written(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, read0)
{
	uint8_t data[] = {-0x01};
	setInput(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {0x00};
	ASSERT_EQ(read(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, read1)
{
	uint8_t data[] = {-0x03, 0x11, 0x22, 0x01, 0x33};
	setInput(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {0x11, 0x22, 0x00, 0x33};
	ASSERT_EQ(read(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, read2)
{
	uint8_t data[] = {-0x02, 0x11, -0x01, -0x01};
	setInput(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {0x11, 0x00, 0x00, 0x00};
	ASSERT_EQ(read(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, read3)
{
	uint8_t data[] = {0x01, 0x01};
	setInput(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {0x01};
	ASSERT_EQ(read(), newVector(expected, SIZE_OF_ARRAY(expected)));
}

TEST_F(ScobsStreamTest, read4)
{
	std::queue<uint8_t> &input = mockStream.input;
	input.push(125);
	for(int i = 1; i <= 125; ++i)
		input.push(i);
	input.push(125);
	for(int i = 126; i <= 250; ++i)
		input.push(i);
	input.push(5);
	for(int i = 251; i <= 255; ++i)
		input.push(i);
	std::vector<uint8_t> actual = read();
	ASSERT_EQ((size_t)255, actual.size());
	for(int i = 1; i <= 255; ++i)
		ASSERT_EQ((uint8_t)i, actual[i - 1]);
}

TEST_F(ScobsStreamTest, read5)
{
	uint8_t data[] = {-0x01, -0x01};
	setInput(data, SIZE_OF_ARRAY(data));
	uint8_t expected[] = {0x00, 0x00};
	ASSERT_EQ(read(), newVector(expected, SIZE_OF_ARRAY(expected)));
}
