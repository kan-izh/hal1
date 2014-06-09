#ifndef MockStream_h
#define MockStream_h

#include <queue>

class MockStream : public Stream
{
public:
	std::vector<uint8_t> written;
	std::queue<uint8_t> input;
	virtual int available()
	{
		return input.size();
	}

	virtual int read()
	{
		if(input.empty())
			return -1;
		uint8_t b = input.front();
		input.pop();
		return b;

	}

	virtual int peek()
	{
		return input.front();
	}

	virtual void flush()
	{
	}

	virtual size_t write(uint8_t b)
	{
		written.push_back(b);
		return 1;
	}
};

#endif
