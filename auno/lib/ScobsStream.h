#ifndef scobs_h
#define scobs_h

#include "Stream.h"

class ScobsStream : public Stream
{
private:
	Stream *const stream;

	static const int BUF_SIZE = 125;
	uint8_t buffer[BUF_SIZE];
	int buffered;

	int8_t stuff;
	bool putZero;

	void finishBlock(bool hasZero);
public:
	ScobsStream(Stream *const stream)
		: stream(stream)
		, buffered(0)
		, stuff(0)
		, putZero(false)
	{
	}

	virtual int available();
	virtual int read();
	virtual int peek();
	virtual void flush();
	virtual size_t write(uint8_t b);
	using Print::write;

};

#endif
