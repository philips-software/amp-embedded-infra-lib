#include "services/util/TracingMessageCommunicationWindowed.hpp"

namespace services
{
    TracingMessageCommunicationWindowed::TracingMessageCommunicationWindowed(MessageCommunicationEncoded& delegate, uint16_t ownWindowSize, Tracer& tracer)
        : MessageCommunicationWindowed(delegate, ownWindowSize)
        , tracer(tracer)
    {}

    void TracingMessageCommunicationWindowed::ReceivedInit(uint16_t otherAvailableWindow)
    {
        tracer.Trace() << "MessageCommunicationWindowed::ReceivedInit otherAvailableWindow: " << otherAvailableWindow;
    }

    void TracingMessageCommunicationWindowed::ReceivedInitResponse(uint16_t otherAvailableWindow)
    {
        tracer.Trace() << "MessageCommunicationWindowed::ReceivedInitResponse otherAvailableWindow: " << otherAvailableWindow;
    }

    void TracingMessageCommunicationWindowed::ReceivedReleaseWindow(uint16_t oldOtherAvailableWindow, uint16_t otherAvailableWindow)
    {
        tracer.Trace() << "MessageCommunicationWindowed::ReceivedReleaseWindow from " << oldOtherAvailableWindow << " to " << otherAvailableWindow;
    }

    void TracingMessageCommunicationWindowed::ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader)
    {
        tracer.Trace() << "MessageCommunicationWindowed::ForwardingReceivedMessage ";

        std::size_t index = 0;

        while (index != reader.Available())
        {
            auto range = reader.PeekContiguousRange(index);
            tracer.Continue() << infra::AsHex(range);
            index += range.size();
        }
    }
}
