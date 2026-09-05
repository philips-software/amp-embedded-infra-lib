#ifndef SERVICES_ECHO_ON_SESAME_HPP
#define SERVICES_ECHO_ON_SESAME_HPP

#include "protobuf/echo/EchoOnStreams.hpp"
#include "services/util/Sesame.hpp"

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
        bool ReceiveOnConfiguredChannel(infra::StreamReaderWithRewinding& reader, SesameChannel channel);
        void ProcessMessage();

    private:
        std::optional<std::size_t> requestedSize;
        SesameChannel requestedChannel = SesameChannel::red;
        bool initialized = false;
        ServiceProxy* sendingProxy = nullptr;
    };
}

#endif
