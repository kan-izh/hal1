#ifndef RingBufferPublisher_h
#define RingBufferPublisher_h

#include <stdint.h>

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
private:
	static const uint8_t highWaterMarkSize = sizeof(uint32_t);
	static const uint16_t bufferArraySize = bufferSize * payloadSize;
	uint8_t buffer[bufferArraySize];
	uint8_t controlMsg[payloadSize];
	uint16_t bufferPos;
	RingBufferOutput *const output;
private:
	uint8_t *cur()
	{
		return buffer + bufferPos;
	}

	const uint8_t *cur() const
	{
		return buffer + bufferPos;
	}

	void sendCurrent() const
	{
		output->write(cur(), payloadSize);
	}

	uint32_t &hwm()
	{
		uint8_t *buf = cur();
		return hwmCast(buf);
	}
	
	const uint32_t hwm() const
	{
		const uint8_t *buf = cur();
		return hwmCast(buf);
	}

	static uint32_t &hwmCast(uint8_t *buf)
	{
		return *reinterpret_cast<uint32_t *>(buf);
	}

	static uint32_t hwmCast(const uint8_t *buf)
	{
		return *reinterpret_cast<const uint32_t *>(buf);
	}

	void clear()
	{
		memset(buffer, 0, bufferArraySize);
	}

	uint8_t *controlCommand(const uint32_t id)
	{
		hwmCast(controlMsg) = id;
		uint8_t *cmdData = controlMsg + highWaterMarkSize;
		memset(cmdData, 0, payloadSize - highWaterMarkSize);
		return cmdData;
	}

	void sendControlCommand()
	{
		output->write(controlMsg, payloadSize);
	}

public:

	RingBufferPublisher(RingBufferOutput *const output)
			: bufferPos(0)
			, output(output)
	{
		setHighWatermark(1);
	}

	void heartbeat() const
	{
		sendCurrent();
	}
	
	void *getSendBuffer()
	{
		return cur() + highWaterMarkSize;
	}

	uint8_t getSendBufferSize()
	{
		return payloadSize - highWaterMarkSize;
	}

	void send()
	{
		sendCurrent();
		uint32_t nextHwm = hwm() + 1;
		if(nextHwm > MAX_HIGH_WATERMARK_VALUE)
		{
			--nextHwm;
			controlCommand(CONTROL_MAX_HIGH_WATERMARK_ID);
			sendControlCommand();
		}
		bufferPos += payloadSize;
		if(bufferPos >= bufferArraySize)
			bufferPos = 0;
		hwm() = nextHwm;
	}

	uint32_t getHighWatermark() const
	{
		return hwm();
	}

	void setHighWatermark(const uint32_t value)
	{
		clear();
		hwm() = value;
	}

	void nak(const uint32_t subscriberHighWatermark)
	{
		if(subscriberHighWatermark > hwm())
		{
			hwmCast(controlCommand(CONTROL_BAD_NAK_ID)) = subscriberHighWatermark;
			sendControlCommand();
			return;
		}
		uint32_t missingMessagesCount = hwm() - subscriberHighWatermark;
		if(missingMessagesCount > bufferSize)
		{
			hwmCast(controlCommand(CONTROL_NAK_BUFFER_OVERFLOW)) = subscriberHighWatermark;
			sendControlCommand();
			return;
		}
		// rewind bufferPos back by the size
		const uint16_t bufferRewindDistance = missingMessagesCount * payloadSize;
		if(bufferRewindDistance <= bufferPos)
			bufferPos -= bufferRewindDistance;
		else
			bufferPos += bufferArraySize - bufferRewindDistance;
		// send all naked messages up to original position
		while (missingMessagesCount--)
		{
			send();
		}
	}
};

#endif //RingBufferPublisher_h
