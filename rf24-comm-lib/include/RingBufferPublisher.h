#ifndef RingBufferPublisher_h
#define RingBufferPublisher_h


#include <stdint.h>
#include "RingBuffer.h"
#include "ByteBuffer.h"

struct RingBufferOutput
{
	virtual void write(uint8_t const *buf, uint8_t len) = 0;
};

template<
		uint16_t bufferSize,
		uint8_t payloadSize
>
class RingBufferPublisher
{
public:
	typedef ByteBuffer<payloadSize> PayloadBuffer;
	typedef typename ByteBuffer<payloadSize>::Accessor PayloadAccessor;
private:
	RingBufferOutput *const output;
	PayloadBuffer buffer[bufferSize];
	PayloadAccessor bufferAccessor;
	PayloadBuffer controlMsg;
private:
	void sendCurrent() const
	{
		output->write(bufferAccessor.getBuf(), payloadSize);
	}

	void sendControlCommand(PayloadAccessor &cmd)
	{
		output->write(cmd.getBuf(), payloadSize);
	}

	void nextBuffer()
	{
		PayloadBuffer *nextBuffer = bufferAccessor.current() + 1;
		if(nextBuffer - buffer >= bufferSize)
			nextBuffer = buffer;
		bufferAccessor.assign(nextBuffer);
	}

public:
	RingBufferPublisher(RingBufferOutput *const output)
			: output(output)
			, bufferAccessor(buffer)
	{
		setHighWatermark(1);
	}

	void heartbeat() const
	{
		sendCurrent();
	}

	PayloadAccessor &getSendBuffer()
	{
		return bufferAccessor;
	}

	void send()
	{
		const uint32_t &hwm = getHighWatermark();
		const uint32_t nextHwm = hwm + 1;
		if(nextHwm > MAX_HIGH_WATERMARK_VALUE)
		{
			setHighWatermark(hwm);
			sendControlCommand(
					PayloadAccessor(&controlMsg)
							.put(CONTROL_MAX_HIGH_WATERMARK_ID));
			return;
		}
		sendCurrent();
		nextBuffer();
		setHighWatermark(nextHwm);
	}

	const uint32_t &getHighWatermark() const
	{
		return bufferAccessor.template get<uint32_t>(0);
	}

	void setHighWatermark(const uint32_t value)
	{
		bufferAccessor.reset();
		bufferAccessor.put(value);
		bufferAccessor.clear();
	}

	void nak(const uint32_t subscriberHighWatermark)
	{
		const uint32_t &hwm = getHighWatermark();
		if(subscriberHighWatermark > hwm)
		{
			sendControlCommand(
					PayloadAccessor(&controlMsg)
							.put(CONTROL_BAD_NAK_ID)
							.put(subscriberHighWatermark));
			return;
		}
		uint32_t missingMessagesCount = hwm - subscriberHighWatermark;
		if(missingMessagesCount > bufferSize)
		{
			sendControlCommand(
					PayloadAccessor(&controlMsg)
							.put(CONTROL_NAK_BUFFER_OVERFLOW)
							.put(subscriberHighWatermark));
			return;
		}
		// rewind bufferPos back by the size
		const uint16_t bufferRewindDistance = static_cast<uint16_t> (missingMessagesCount);
		PayloadBuffer *rewindTo = bufferAccessor.current();
		rewindTo -= bufferRewindDistance;
		if(rewindTo < buffer)
			rewindTo += bufferSize;
		bufferAccessor.assign(rewindTo);
		// send all naked messages up to original position
		while (missingMessagesCount--)
		{
			sendCurrent();
			nextBuffer();
		}
	}

	void sendNak(const uint32_t &hwm)
	{
		sendControlCommand(
				PayloadAccessor(&controlMsg)
						.put(CONTROL_REQUEST_NAK)
						.put(hwm)
		);
	}
};

#endif //RingBufferPublisher_h
