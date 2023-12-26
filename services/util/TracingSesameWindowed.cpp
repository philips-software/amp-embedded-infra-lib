#include "services/util/TracingSesameWindowed.hpp"

namespace services
{
    TracingSesameWindowed::TracingSesameWindowed(SesameEncoded& delegate, Tracer& tracer)
        : SesameWindowed(delegate)
        , tracer(tracer)
    {}

    void TracingSesameWindowed::ReceivedInit(uint16_t newWindow)
    {
        tracer.Trace() << "SesameWindowed::ReceivedInit newWindow: " << newWindow;
    }

    void TracingSesameWindowed::ReceivedInitResponse(uint16_t newWindow)
    {
        tracer.Trace() << "SesameWindowed::ReceivedInitResponse newWindow: " << newWindow;
    }

    void TracingSesameWindowed::ReceivedReleaseWindow(uint16_t oldWindow, uint16_t newWindow)
    {
        tracer.Trace() << "SesameWindowed::ReceivedReleaseWindow from " << oldWindow << " to " << newWindow;
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

    void TracingSesameWindowed::SendingInit(uint16_t newWindow)
    {
        tracer.Trace() << "SesameWindowed::SendingInit newWindow: " << newWindow;
    }

    void TracingSesameWindowed::SendingInitResponse(uint16_t newWindow)
    {
        tracer.Trace() << "SesameWindowed::SendingInitResponse newWindow: " << newWindow;
    }

    void TracingSesameWindowed::SendingReleaseWindow(uint16_t deltaWindow)
    {
        tracer.Trace() << "SesameWindowed::SendingReleaseWindow deltaWindow: " << deltaWindow;
    }

    void TracingSesameWindowed::SendingMessage(infra::StreamWriter& writer)
    {
        tracer.Trace() << "SesameWindowed::SendingMessage";
    }
}
