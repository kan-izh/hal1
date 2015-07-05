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
        uint16_t bufferSize,
        uint8_t payloadSize
>
class CommChannel
{
private:
    static const int defaultTimeoutMicros = 10000;
public:
    typedef uint16_t Sequence;
    typedef ByteBuffer<payloadSize - sizeof(Sequence)> Buffer;
    typedef typename Buffer::Accessor BufferAccessor;
    struct Receiver
    {
        virtual void receive(typename ByteBuffer<payloadSize>::Accessor &data) = 0;
    };

public:
    CommChannel(TimeSource &timeSource, CommChannelOutput &output, Receiver &receiver)
            : timeSource(timeSource)
            , sender(output)
            , sequence(0)
            , ackedSequence(0)
            , receiver(receiver)
            , timeoutMicros(defaultTimeoutMicros)
    { }

    BufferAccessor currentFrame()
    {
        BufferAccessor accessor(bufferAt(sequence));
        accessor.put(timeSource.currentMicros());
        return accessor;
    }
    
    void sendFrame(BufferAccessor &accessor)
    {
        accessor.clear();
        write(accessor, sequence);
        ++sequence;
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
        const Sequence &senderSequence = accessor.template take<Sequence>();
        const uint32_t &timestamp = accessor.template take<uint32_t>();
        receiver.receive(accessor);
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
        return 0;
    }

private:
    Buffer *bufferAt(const Sequence seq)
    {
        return buffer + seq % bufferSize;
    }

    void resendNotAckedFrames(uint32_t currentMicros)
    {
        for(Sequence seq = ackedSequence; sequence != seq; ++seq)
        {
            BufferAccessor accessor(bufferAt(seq));
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
        payloadAccessor
                .put(seq)
                .template append<payloadSize - sizeof(Sequence)>(accessor)
                .clear();
        sender.write(payload.getBuf(), payloadSize);
    }
private:
    TimeSource &timeSource;
    CommChannelOutput &sender;
    Receiver &receiver;

    Buffer buffer[bufferSize];
    ByteBuffer<payloadSize> inbound;
    Sequence sequence;
    Sequence ackedSequence;
    uint32_t timeoutMicros;    
};
#endif //HAL1_COMMCHANNEL_H
