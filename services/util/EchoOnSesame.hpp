#ifndef SERVICES_ECHO_ON_SESAME_HPP
#define SERVICES_ECHO_ON_SESAME_HPP

#include "protobuf/echo/Echo.hpp"
#include "services/util/Sesame.hpp"

namespace services
{
    class EchoOnSesame
        : public EchoOnStreams
        , public SesameObserver
    {
    public:
        EchoOnSesame(Sesame& subject, services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        void Reset();

        // Implementation of SesameObserver
        void Initialized() override;
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

    protected:
        // Implementation of EchoOnStreams
        void RequestSendStream(std::size_t size) override;

    private:
        void ProcessMessage();

    private:
        infra::Optional<std::size_t> requestedSize;
        bool initialized = false;
    };
}

#endif
