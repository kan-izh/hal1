#ifndef RingBufferPublisher_h
#define RingBufferPublisher_h


#include <stdint.h>
#include "ByteBuffer.h"

static uint32_t const CONTROL_BAD_NAK_ID = (uint32_t) (-0x1);
static uint32_t const CONTROL_NAK_BUFFER_OVERFLOW = (uint32_t) (-0x2);
static uint32_t const CONTROL_MAX_HIGH_WATERMARK_ID = (uint32_t) (-0x3);
static uint32_t const MAX_HIGH_WATERMARK_VALUE = (uint32_t) (-0x100);

struct RingBufferOutput
{
	virtual void write(uint8_t const *buf, uint8_t len) = 0;
};

template<
		uint16_t bufferSize,
		uint8_t payloadSize = 32
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
	PayloadAccessor &cur()
	{
		return bufferAccessor;
	}

	const PayloadAccessor &cur() const
	{
		return bufferAccessor;
	}

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
		bufferAccessor.set(nextBuffer);
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
		return cur();
	}

	void send()
	{
		const uint32_t nextHwm = getHighWatermark() + 1;
		if(nextHwm > MAX_HIGH_WATERMARK_VALUE)
		{
			setHighWatermark(getHighWatermark());
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
		const PayloadAccessor &accessor = cur();
		const uint32_t &value = accessor.template get<uint32_t>(0);
		return value;
	}

	void setHighWatermark(const uint32_t value)
	{
		cur().reset();
		cur().put(value);
		cur().clear();
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
		bufferAccessor.set(rewindTo);
		// send all naked messages up to original position
		while (missingMessagesCount--)
		{
			sendCurrent();
			nextBuffer();
		}
	}
};

#endif //RingBufferPublisher_h
