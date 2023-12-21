#ifndef SERVICES_TRACING_MESSAGE_COMMUNICATION_WINDOWED_HPP
#define SERVICES_TRACING_MESSAGE_COMMUNICATION_WINDOWED_HPP

#include "services/util/MessageCommunicationWindowed.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingMessageCommunicationWindowed
        : public MessageCommunicationWindowed
    {
    public:
        TracingMessageCommunicationWindowed(MessageCommunication& delegate, uint16_t ownWindowSize, Tracer& tracer);

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
