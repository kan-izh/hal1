#include "ScobsStream.h"

void ScobsStream::finishBlock(bool hasZero)
{
	if(buffered == 0 && !hasZero)
		return;
	const int stuff = hasZero ? -(buffered + 1) : buffered;
	stream->write(stuff);
	stream->write(buffer, buffered);
	buffered = 0;
}

int ScobsStream::available()
{
	return 0;
}

int ScobsStream::read()
{
	if(stuff == 0)
	{
		int read = stream->read();
		if(read == -1)
			return -1;
		int8_t readByte = read;
		putZero = readByte < 0;
		stuff = readByte < 0 ? -readByte : readByte;
	}
	stuff--;
	if(stuff == 0 && putZero)
		return 0;
	return stream->read();
}

int ScobsStream::peek()
{
	return -1;
}

void ScobsStream::flush()
{
	finishBlock(false);
	stream->flush();
}

size_t ScobsStream::write(uint8_t b)
{
	if(b == 0)
	{
		finishBlock(true);
	}
	else
	{
		buffer[buffered++] = b;
		if(buffered == BUF_SIZE)
			finishBlock(false);
	}
	return 1;
}
