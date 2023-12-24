#include "services/util/TracingSesameWindowed.hpp"

namespace services
{
    TracingSesameWindowed::TracingSesameWindowed(SesameEncoded& delegate, uint16_t ownWindowSize, Tracer& tracer)
        : SesameWindowed(delegate, ownWindowSize)
        , tracer(tracer)
    {}

    void TracingSesameWindowed::ReceivedInit(uint16_t otherAvailableWindow)
    {
        tracer.Trace() << "SesameWindowed::ReceivedInit otherAvailableWindow: " << otherAvailableWindow;
    }

    void TracingSesameWindowed::ReceivedInitResponse(uint16_t otherAvailableWindow)
    {
        tracer.Trace() << "SesameWindowed::ReceivedInitResponse otherAvailableWindow: " << otherAvailableWindow;
    }

    void TracingSesameWindowed::ReceivedReleaseWindow(uint16_t oldOtherAvailableWindow, uint16_t otherAvailableWindow)
    {
        tracer.Trace() << "SesameWindowed::ReceivedReleaseWindow from " << oldOtherAvailableWindow << " to " << otherAvailableWindow;
    }

    void TracingSesameWindowed::ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader)
    {
        tracer.Trace() << "SesameWindowed::ForwardingReceivedMessage ";

        std::size_t index = 0;

        while (index != reader.Available())
        {
            auto range = reader.PeekContiguousRange(index);
            tracer.Continue() << infra::AsHex(range);
            index += range.size();
        }
    }
}
