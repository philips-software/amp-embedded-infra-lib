#ifndef SERVICES_TRACING_MESSAGE_COMMUNICATION_WINDOWED_HPP
#define SERVICES_TRACING_MESSAGE_COMMUNICATION_WINDOWED_HPP

#include "services/tracer/Tracer.hpp"
#include "services/util/SesameWindowed.hpp"

namespace services
{
    class TracingSesameWindowed
        : public SesameWindowed
    {
    public:
        TracingSesameWindowed(SesameEncoded& delegate, uint16_t ownWindowSize, Tracer& tracer);

    protected:
        void ReceivedInit(uint16_t otherAvailableWindow) override;
        void ReceivedInitResponse(uint16_t otherAvailableWindow) override;
        void ReceivedReleaseWindow(uint16_t oldOtherAvailableWindow, uint16_t otherAvailableWindow) override;
        void ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader) override;

    private:
        Tracer& tracer;
    };
}

#endif
