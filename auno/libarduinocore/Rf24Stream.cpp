#include "Rf24Stream.h"

Rf24Stream::Rf24Stream(RF24 *rf24)
	: rf24(rf24)
	, outPos(0)
	, inPos(0)
{
	bufferSize = rf24->getPayloadSize();
	outBuffer = new uint8_t[bufferSize];
	inBuffer = new uint8_t[bufferSize];
}

Rf24Stream::~Rf24Stream()
{
	flush();
	delete []outBuffer;
	delete []inBuffer;
}

size_t Rf24Stream::write(uint8_t b)
{
	outBuffer[outPos++] = b;
	if(outPos == bufferSize)
		return writeBuffer() ? 1 : 0;
	return 1;
}

bool Rf24Stream::writeBuffer()
{
	rf24->stopListening();
	bool success = rf24->write(outBuffer, outPos);
    rf24->txStandBy(1000);
	rf24->startListening();
	outPos = 0;
	return success;
}

void Rf24Stream::flush()
{
	if(outPos == 0)
		return;
	writeBuffer();
}

int Rf24Stream::available()
{
	return rf24->available() ? bufferSize : 0;
}

int Rf24Stream::read()
{
	if(inPos == 0)
		readBuffer();
	return inBuffer[--inPos];
}

void Rf24Stream::readBuffer()
{
	rf24->read(inBuffer, bufferSize);
	inPos = bufferSize;
}

int Rf24Stream::peek()
{
	return inPos == 0 ? -1 : inBuffer[inPos];
}
