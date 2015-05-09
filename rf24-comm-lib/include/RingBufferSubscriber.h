#ifndef HAL1_RINGBUFFERSUBSCRIBER_H
#define HAL1_RINGBUFFERSUBSCRIBER_H

#include <stdint.h>
#include "RingBuffer.h"
#include "ByteBuffer.h"

template<
		uint8_t payloadSize = 32
>
class RingBufferSubscriber
{
public:
	typedef ByteBuffer<payloadSize> PayloadBuffer;
	typedef typename ByteBuffer<payloadSize>::Accessor PayloadAccessor;
	struct Handler
	{
		virtual void handle(uint32_t messageId, uint8_t contentId, PayloadAccessor &input) = 0;
		virtual void handleNak(uint32_t hwm, uint32_t sequence) = 0;
		virtual void nak(const uint32_t &subscriberHighWatermark) = 0;
		virtual void recover(uint32_t hwm) = 0;
	};
private:
	Handler *handler;
	PayloadBuffer buffer;
	PayloadAccessor accessor;
	uint32_t hwm;


public:
	RingBufferSubscriber(Handler *handler)
			: handler(handler)
			, accessor(&buffer)
			, hwm(0)
	{
	}

	uint8_t *getBuf()
	{
		return buffer.getBuf();
	}
	
	uint8_t getBufSize()
	{
		return payloadSize;
	}

	void process()
	{
		const uint32_t &controlId = accessor.template take<uint32_t>();
		if (controlId <= MAX_HIGH_WATERMARK_VALUE)
		{
			processMessage(controlId);
		}
		else
		{
			processControl(controlId);
		}
		accessor.reset();
	}

private:

	void processControl(const uint32_t &controlId)
	{
		switch (controlId)
		{
			case CONTROL_REQUEST_NAK:
			{
				const uint32_t &subscriberHighWatermark = accessor.template take<uint32_t>();
				handler->nak(subscriberHighWatermark);
				break;
			}
			case CONTROL_NAK_BUFFER_OVERFLOW:
			{
				hwm = 0;
				break;
			}
			default:
				break;
		}
	}

	void processMessage(const uint32_t &sequence)
	{
		if (hwm == 0)
		{
			hwm = sequence;
			handler->recover(hwm);
		}

		if(hwm == sequence)
		{
			handleMessage();
			hwm = sequence + 1;
		}
		else if(hwm < sequence)
		{
			requestNak(sequence);
		}
	}

	void handleMessage()
	{
		const uint8_t &id = accessor.template take<uint8_t>();
		handler->handle(this->hwm, id, accessor);
	}
	
	void requestNak(const uint32_t &sequence)
	{
		handler->handleNak(this->hwm, sequence);
	}
};


#endif //HAL1_RINGBUFFERSUBSCRIBER_H
