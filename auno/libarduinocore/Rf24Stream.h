#ifndef Rf24Stream_h
#define Rf24Stream_h

#include "Arduino.h"
#include "RF24.h"
#include "Stream.h"

class Rf24Stream : public Stream
{
private:
		RF24 *rf24;
		uint8_t *outBuffer, *inBuffer;
		uint8_t outPos, inPos, bufferSize;
		bool writeBuffer();
		void readBuffer();
public:
		Rf24Stream(RF24 *rf24);
		virtual ~Rf24Stream();
		virtual int available();
		virtual int read();
		virtual int peek();
		virtual void flush();
		virtual size_t write(uint8_t b);
		using Print::write;
};
#endif //Rf24Stream_h
