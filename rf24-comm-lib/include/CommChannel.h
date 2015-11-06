#ifndef HAL1_COMMCHANNEL_H
#define HAL1_COMMCHANNEL_H


#include <stdint.h>
#include "ByteBuffer.h"

struct TimeSource
{
	virtual uint32_t currentMicros() = 0;
};

struct CommChannelOutput
{
	virtual void write(uint8_t const *buf, uint8_t len) = 0;
};

template<
		uint16_t outboundBufferSize,
		uint8_t payloadSize
>
class CommChannel
{
public:
	typedef uint16_t Sequence;
private:
	typedef uint8_t Command;
	static const size_t outboundBufferElemSize = payloadSize - sizeof(Command) - sizeof(Sequence);
	static const size_t inboundBufferElemSize = payloadSize + sizeof(uint8_t) - sizeof(Command) - sizeof(Sequence);
	static const size_t inboundBufferSize = outboundBufferSize;
public:
	typedef ByteBuffer<outboundBufferElemSize> Buffer;
	typedef typename ByteBuffer<inboundBufferElemSize>::Accessor InboundAccessor;
	typedef typename Buffer::Accessor BufferAccessor;
	struct Receiver
	{
		typedef typename ByteBuffer<payloadSize>::Accessor Accessor;
		virtual void receive(Accessor &data) = 0;
		virtual void restart() = 0;
	};

private:
	static const int defaultTimeoutMicros = 10000;
	static const Command cmd_data = 1;
	static const Command cmd_ack = 2;
	static const Command cmd_nak = 3;
	static const Command cmd_nakOverflow = 4;
	static const Command cmd_firstData = 5;
	static const Command cmd_firstAck = 6;
	static const uint8_t received_firstData = 0xFF;
	template<
	        typename Elem,
			uint16_t size
	>
	struct RingBuffer
	{
		RingBuffer()
				: head(0)
				, tail(0)
		{ }

		Elem *bufferAt(const Sequence seq)
		{
			return buffer + seq % size;
		}

		Elem buffer[size];
		Sequence head;
		Sequence tail;
	};
public:
	CommChannel(TimeSource &timeSource, CommChannelOutput &output, Receiver &receiver)
			: timeSource(timeSource)
			, sender(output)
			, receiver(receiver)
			, timeoutMicros(defaultTimeoutMicros)
			, joined(false)
			, initialised(false)
			, shouldAck(false)
	{ }

	BufferAccessor currentFrame()
	{
		BufferAccessor accessor(outbound.bufferAt(outbound.head));
		accessor.put(timeSource.currentMicros());
		return accessor;
	}

	void sendFrame(BufferAccessor &accessor)
	{
		accessor.clear();
		write(accessor, outbound.head);
		++outbound.head;
	}

	uint8_t *getBuf()
	{
		return inbound.getBuf();
	}

	uint8_t getBufSize()
	{
		return payloadSize;
	}

	void processBuf()
	{
		typename ByteBuffer<payloadSize>::Accessor accessor(&inbound);
		const Command &command = accessor.template take<Command>();
		switch (command)
		{
			case cmd_firstAck:
				inboundAck(accessor, true);
				break;
			case cmd_ack:
				inboundAck(accessor, false);
				break;
			case cmd_firstData:
				inboundData(accessor, true);
				break;
			case cmd_data:
				inboundData(accessor, false);
				break;
			case cmd_nak:
				inboundNak(accessor);
				break;
			case cmd_nakOverflow:
				inboundNakOverflow(accessor);
				break;
			default:
				break;
		}
	}

	uint32_t getTimeoutMicros() const
	{
		return timeoutMicros;
	}

	void setTimeoutMicros(uint32_t timeoutMicros)
	{
		CommChannel::timeoutMicros = timeoutMicros;
	}

	uint32_t  processIdle()
	{
		uint32_t currentMicros = timeSource.currentMicros();
		resendNotAckedFrames(currentMicros);
		consumeInbound();
		return 0;
	}

private:

	void inboundNakOverflow(typename ByteBuffer<payloadSize>::Accessor &accessor)
	{
		const Sequence &badNak = accessor.template take<Sequence>();
		if (badNak != inboundBuffer.tail)
		{
			return;// not our nak
		}
		joined = false;
	}

	void inboundNak(typename ByteBuffer<payloadSize>::Accessor &accessor)
	{
		const Sequence &nakFrom = accessor.template take<Sequence>();
		const Sequence nakSize = outbound.head - nakFrom;
		if(nakSize > outboundBufferSize)
		{
			ByteBuffer<payloadSize> payload;
			typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
			payloadAccessor.put(cmd_nakOverflow);
			payloadAccessor.put(nakFrom);
			payloadAccessor.clear();
			sender.write(payload.getBuf(), payloadSize);
			return;
		}
		int bitNumber = -1;//we send the nakFrom always, the mask for frames after.
		uint8_t mask = 0, bit = 1;
		for(Sequence seq = nakFrom;
				seq != outbound.head
				&& accessor.getOffset() < payloadSize;
				++seq, ++bitNumber, bit <<= 1)
		{
			if (bitNumber % 8 == 0)
			{//first bit in byte
				mask = accessor.template take<uint8_t>();
				bit = 1;
			}
			if (bitNumber == -1 || !(mask & bit))
			{
				BufferAccessor data(outbound.bufferAt(seq));
				write(data, seq);
			}
		}
	}

	void inboundAck(typename ByteBuffer<payloadSize>::Accessor &accessor, const bool firstAck)
	{
		const Sequence &acked = accessor.template take<Sequence>();
		outbound.tail = acked;
		if(firstAck)
		{
			initialised = true;
		}
	}

	void inboundData(typename ByteBuffer<payloadSize>::Accessor &accessor, const bool firstData)
	{
		const Sequence &senderSequence = accessor.template take<Sequence>();
		if (!joined || firstData)
		{
			receiver.restart();
			joined = true;
			for(; inboundBuffer.tail != inboundBuffer.head; ++inboundBuffer.tail)
			{
				InboundAccessor bufAccessor(inboundBuffer.bufferAt(inboundBuffer.tail));
				uint8_t &received = bufAccessor.template take<uint8_t>();
				received = 0;
			}
			inboundBuffer.tail = inboundBuffer.head  = senderSequence;
		}
		const Sequence posT = senderSequence - inboundBuffer.tail;
		const Sequence posH = inboundBuffer.head - senderSequence;
		if (posT < inboundBufferSize)
		{
			InboundAccessor bufAccessor(inboundBuffer.bufferAt(senderSequence));
			uint8_t &received = bufAccessor.template take<uint8_t>();
			if (received == 0)
			{
				bufAccessor.template append<payloadSize>(accessor, accessor.getOffset());
				Sequence size = inboundBuffer.head - inboundBuffer.tail;
				if (posT >= size)
				{// frame is outside, so stretch the buffer
					inboundBuffer.head = senderSequence + Sequence(1);
				}
				else
				{// frame falls in between tail and head
				}

				if (firstData)
				{
					received = received_firstData;
				}
				else
				{
					received = 1;
				}
			}
			else
			{// already received, but not consumed yet as something is missing, ignoring dupe.
			}
		}
		else
		{
			if(posH < inboundBufferSize)
			{//received consumed frame, ignoring dupe.
				shouldAck = true;
			}
			else
			{// overflow, sending nak
				sendNak();
			}
		}
	}

	void sendNak()
	{
		ByteBuffer<payloadSize> payload;
		typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
		payloadAccessor.put(cmd_nak);
		payloadAccessor.put(inboundBuffer.tail);
		uint8_t mask = 0, bit = 1;
		int bitNumber = 0;
		for (Sequence seq = inboundBuffer.tail + Sequence(1U);
				seq != inboundBuffer.head
						&& payloadAccessor.getOffset() < payloadSize;
				++seq, ++bitNumber)
		{
			InboundAccessor bufAccessor(inboundBuffer.bufferAt(seq));
			const uint8_t &received = bufAccessor.template take<uint8_t>();
			mask |= received != 0 ? bit : 0;
			bit <<= 1;
			if (bitNumber % 8 == 7)
			{//last bit in byte
				payloadAccessor.put(mask);
				mask = 0;
				bit = 1;
			}
		}
		if (mask != 0 && payloadAccessor.getOffset() < payloadSize)
		{
			payloadAccessor.put(mask);
		}
		payloadAccessor.clear();
		sender.write(payload.getBuf(), payloadSize);
	}

	void consumeInbound()
	{
		bool firstData = false;
		for(; inboundBuffer.tail != inboundBuffer.head; ++inboundBuffer.tail)
		{
			InboundAccessor bufAccessor(inboundBuffer.bufferAt(inboundBuffer.tail));
			uint8_t &received = bufAccessor.template take<uint8_t>();
			if(received == 0)
			{
				sendNak();
				break;
			}
			else if(received == received_firstData)
			{
				firstData = true;
			}
			received = 0;
			shouldAck = true;
			ByteBuffer<payloadSize> tempPayload;
			typename ByteBuffer<payloadSize>::Accessor tempAccessor(&tempPayload);
			tempAccessor.template take<Command>();
			tempAccessor.template take<Sequence>();
			tempAccessor.template append<inboundBufferElemSize>(bufAccessor, bufAccessor.getOffset());
			tempAccessor.reset();
			tempAccessor.template take<Command>();
			tempAccessor.template take<Sequence>();
			const uint32_t timestamp = tempAccessor.template take<uint32_t >();
			receiver.receive(tempAccessor);
		}
		if(shouldAck)
		{
			sendAck(inboundBuffer.tail, firstData);
			shouldAck = false;
		}
	}

	void sendAck(Sequence consumedSeq, const bool firstData)
	{
		ByteBuffer<payloadSize> payload;
		typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
		payloadAccessor.put(firstData ? cmd_firstAck : cmd_ack);
		payloadAccessor.put(consumedSeq);
		payloadAccessor.clear();
		sender.write(payload.getBuf(), payloadSize);
	}

	void resendNotAckedFrames(uint32_t currentMicros)
	{
		for(Sequence seq = outbound.tail; outbound.head != seq; ++seq)
		{
			BufferAccessor accessor(outbound.bufferAt(seq));
			uint32_t &frameTimestamp = accessor.template take<uint32_t>();
			uint32_t delay = currentMicros - frameTimestamp;
			if(delay >= timeoutMicros)
			{
				frameTimestamp = currentMicros;
				write(accessor, seq);
			}
		}
	}

	void write(BufferAccessor &accessor, const Sequence seq) const
	{
		ByteBuffer<payloadSize> payload;
		typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
		payloadAccessor.put(initialised ? cmd_data : cmd_firstData);
		payloadAccessor.put(seq);
		payloadAccessor.template append<outboundBufferElemSize>(accessor);
		sender.write(payload.getBuf(), payloadSize);
	}
private:
	bool joined, initialised, shouldAck;
	TimeSource &timeSource;
	CommChannelOutput &sender;
	Receiver &receiver;

	ByteBuffer<payloadSize> inbound;
	RingBuffer<Buffer, outboundBufferSize> outbound;
	RingBuffer<ByteBuffer<inboundBufferElemSize>, inboundBufferSize> inboundBuffer;
	uint32_t timeoutMicros;
};
#endif //HAL1_COMMCHANNEL_H
