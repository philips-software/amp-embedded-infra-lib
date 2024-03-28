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
        TracingSesameWindowed(SesameEncoded& delegate, Tracer& tracer);

    protected:
        void ReceivedInit(uint16_t newWindow) override;
        void ReceivedInitResponse(uint16_t newWindow) override;
        void ReceivedReleaseWindow(uint16_t oldWindow, uint16_t newWindow) override;
        void ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader) override;

        void SendingInit(uint16_t newWindow) override;
        void SendingInitResponse(uint16_t newWindow) override;
        void SendingReleaseWindow(uint16_t deltaWindow) override;
        void SendingMessage(infra::StreamWriter& writer) override;
        void SettingOperational(infra::Optional<std::size_t> requestedSize, uint16_t releasedWindow, uint16_t otherWindow) override;

    private:
        Tracer& tracer;
    };
}

#endif
