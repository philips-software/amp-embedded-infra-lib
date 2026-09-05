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
        template<std::size_t MaxMessageSize, uint8_t SplitBuffers = 2>
        struct WithMaxMessageSize;

        TracingSesameWindowed(infra::BoundedDeque<uint8_t>& receivedMessage, uint8_t splitBuffers, SesameEncoded& delegate, Tracer& tracer, SesameInitializer& sesameInitializer = immediatelyGranted);

    protected:
        void ReceivedInit(uint16_t newWindow) override;
        void ReceivedInitResponse(uint16_t newWindow) override;
        void ReceivedReleaseWindow(uint16_t oldWindow, uint16_t newWindow) override;
        void ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader) override;

        void SendingInit(uint16_t newWindow) override;
        void SendingInitResponse(uint16_t newWindow) override;
        void SendingReleaseWindow(uint16_t deltaWindow) override;
        void SendingMessage(infra::StreamWriter& writer, SesameChannel channel) override;
        void SettingOperational(std::optional<std::size_t> requestedSize, uint16_t releasedWindow, uint16_t otherWindow) override;

    private:
        Tracer& tracer;
    };

    template<std::size_t MaxMessageSize, uint8_t SplitBuffers>
    struct TracingSesameWindowed::WithMaxMessageSize
        : infra::WithStorage<TracingSesameWindowed, infra::BoundedDeque<uint8_t>::WithMaxSize<receiveBufferSize<MaxMessageSize, SplitBuffers>>>
    {
        static_assert(SplitBuffers >= 2, "SesameWindowed requires at least 2 receive buffers");

        WithMaxMessageSize(SesameEncoded& delegate, Tracer& tracer, SesameInitializer& sesameInitializer = immediatelyGranted)
            : infra::WithStorage<TracingSesameWindowed, infra::BoundedDeque<uint8_t>::WithMaxSize<receiveBufferSize<MaxMessageSize, SplitBuffers>>>::WithStorage(SplitBuffers, delegate, tracer, sesameInitializer)
        {}
    };
}

#endif
