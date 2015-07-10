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
	static const size_t internalOutboundBufferSize = payloadSize - sizeof(Command) - sizeof(Sequence);
public:
	typedef ByteBuffer<internalOutboundBufferSize> Buffer;
	typedef typename Buffer::Accessor BufferAccessor;
	struct Receiver
	{
		virtual void receive(typename ByteBuffer<payloadSize>::Accessor &data) = 0;
		virtual void restart() = 0;
	};

private:
	static const int defaultTimeoutMicros = 10000;
	static const Command cmd_data = 1;
	static const Command cmd_ack = 2;
	static const Command cmd_nak = 3;
	struct RingBuffer
	{
		RingBuffer()
				: sequence(0)
				, ackedSequence(0)
		{ }

		Buffer *bufferAt(const Sequence seq)
		{
			return buffer + seq % outboundBufferSize;
		}

		Buffer buffer[outboundBufferSize];
		Sequence sequence;
		Sequence ackedSequence;
	};
public:
	CommChannel(TimeSource &timeSource, CommChannelOutput &output, Receiver &receiver)
			: timeSource(timeSource)
			, sender(output)
			, inboundSequence(0)
			, inboundAckedSequence(0)
			, receiver(receiver)
			, timeoutMicros(defaultTimeoutMicros)
			, joined(false)
	{ }

	BufferAccessor currentFrame()
	{
		BufferAccessor accessor(outbound.bufferAt(outbound.sequence));
		accessor.put(timeSource.currentMicros());
		return accessor;
	}

	void sendFrame(BufferAccessor &accessor)
	{
		accessor.clear();
		write(accessor, outbound.sequence);
		++outbound.sequence;
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
			case cmd_ack:
				inboundAck(accessor);
				break;
			case cmd_data:
				inboundData(accessor);
				break;
			case cmd_nak:
				inboundNak(accessor);
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
		sendAcks();
		return 0;
	}

private:

	void inboundNak(typename ByteBuffer<payloadSize>::Accessor &accessor)
	{
		const Sequence &nakFrom = accessor.template take<Sequence>();
		for(Sequence seq = nakFrom; seq != outbound.sequence; ++seq)
		{
			BufferAccessor data(outbound.bufferAt(seq));
			write(data, seq);
		}
	}

	void inboundAck(typename ByteBuffer<payloadSize>::Accessor &accessor)
	{
		const Sequence &acked = accessor.template take<Sequence>();
		outbound.ackedSequence = acked;
	}

	void inboundData(typename ByteBuffer<payloadSize>::Accessor &accessor)
	{
		const Sequence &senderSequence = accessor.template take<Sequence>();
		if (!joined)
		{
			receiver.restart();
			joined = true;
			inboundSequence = senderSequence;
		}
		if (senderSequence == inboundSequence)
		{
			const uint32_t &timestamp = accessor.template take<uint32_t>();
			receiver.receive(accessor);
			++inboundSequence;
		}
		else
		{
			ByteBuffer<payloadSize> payload;
			typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
			payloadAccessor.put(cmd_nak);
			payloadAccessor.put(inboundSequence);
			payloadAccessor.clear();
			sender.write(payload.getBuf(), payloadSize);
		}
	}

	void sendAcks()
	{
		if (inboundAckedSequence != inboundSequence)
		{
			ByteBuffer<payloadSize> payload;
			typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
			payloadAccessor.put(cmd_ack);
			payloadAccessor.put(inboundSequence);
			payloadAccessor.clear();
			sender.write(payload.getBuf(), payloadSize);
			inboundAckedSequence = inboundSequence;
		}
	}

	void resendNotAckedFrames(uint32_t currentMicros)
	{
		for(Sequence seq = outbound.ackedSequence; outbound.sequence != seq; ++seq)
		{
			BufferAccessor accessor(outbound.bufferAt(seq));
			const uint32_t &frameTimestamp = accessor.template take<uint32_t>();
			uint32_t delay = currentMicros - frameTimestamp;
			if(delay >= timeoutMicros)
			{
				write(accessor, seq);
			}
		}
	}

	void write(BufferAccessor &accessor, const Sequence seq) const
	{
		ByteBuffer<payloadSize> payload;
		typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
		payloadAccessor.put(cmd_data);
		payloadAccessor.put(seq);
		payloadAccessor.template append<internalOutboundBufferSize>(accessor);
		sender.write(payload.getBuf(), payloadSize);
	}
private:
	bool joined;
	TimeSource &timeSource;
	CommChannelOutput &sender;
	Receiver &receiver;

	ByteBuffer<payloadSize> inbound;
	Sequence inboundSequence;
	Sequence inboundAckedSequence;
	RingBuffer outbound;
	uint32_t timeoutMicros;
};
#endif //HAL1_COMMCHANNEL_H
