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
            , receiver(receiver)
    { }

    BufferAccessor currentFrame()
    {
        return BufferAccessor(currentOutbound());
    }
    
    void sendFrame(BufferAccessor &accessor)
    {
        Buffer *outbound = currentOutbound();
        ByteBuffer<payloadSize> payload;
        typename ByteBuffer<payloadSize>::Accessor payloadAccessor(&payload);
        payloadAccessor
                .put(sequence)
                .template append<payloadSize - sizeof(Sequence)>(accessor)
                .clear();
        sender.write(payload.getBuf(), payloadSize);
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
        receiver.receive(accessor);
    }

private:
    Buffer *currentOutbound() { return buffer + sequence % bufferSize; }
private:
    TimeSource &timeSource;
    CommChannelOutput &sender;
    Receiver &receiver;

    Buffer buffer[bufferSize];
    ByteBuffer<payloadSize> inbound;
    Sequence sequence;
    Sequence ackedSequence;
};
#endif //HAL1_COMMCHANNEL_H
