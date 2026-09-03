#ifndef SERVICES_ECHO_ON_SESAME_HPP
#define SERVICES_ECHO_ON_SESAME_HPP

#include "protobuf/echo/EchoOnStreams.hpp"
#include "services/util/Sesame.hpp"
#include <array>
#include <utility>

namespace services
{
    class EchoOnSesame
        : public EchoOnStreams
        , public EchoInitialization
        , private SesameObserver
    {
    public:
        EchoOnSesame(Sesame& subject, services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        void Reset();
        void SetServiceChannel(uint32_t serviceId, SesameChannel channel);
        SesameChannel ServiceChannel(uint32_t serviceId) const;

        // Implementation of SesameObserver
        void Initialized() override;
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer, SesameChannel channel) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, SesameChannel channel) override;

    protected:
        // Implementation of EchoOnStreams
        void RequestSendStream(std::size_t size) override;
        void ResetReading() override;
        void SendingProxySelected(ServiceProxy& proxy) override;

    private:
        bool ReceiveOnConfiguredChannel(infra::StreamReaderWithRewinding& reader, SesameChannel channel) const;
        void ProcessMessage();

    private:
        static constexpr std::size_t numberOfServiceChannels = 16;
        using ServiceChannelMap = std::array<std::pair<uint32_t, SesameChannel>, numberOfServiceChannels>;

        std::optional<std::size_t> requestedSize;
        SesameChannel requestedChannel = SesameChannel::red;
        bool initialized = false;
        ServiceProxy* sendingProxy = nullptr;
        ServiceChannelMap serviceChannels{};
        std::size_t numberOfMappedServices = 0;
    };
}

#endif
