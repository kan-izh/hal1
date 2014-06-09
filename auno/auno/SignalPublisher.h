#ifndef signals_h
#define signals_h

#include "ScobsStream.h"
#include <pb_encode.h>

class SignalPublisher
{
private:
	Stream *const stream;
	ScobsStream scobsStream;
	friend bool serial_writer(pb_ostream_t *stream, const uint8_t *buf, size_t count);
public:
	SignalPublisher(Stream *const stream)
		: stream(stream)
		, scobsStream(stream)
	{
	}

	void send();
};

#endif
