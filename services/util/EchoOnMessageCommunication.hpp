#ifndef SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_HPP
#define SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_HPP

#include "protobuf/echo/Echo.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class EchoOnMessageCommunication
        : public EchoOnStreams
        , public MessageCommunicationObserver
    {
    public:
        EchoOnMessageCommunication(MessageCommunication& subject, EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of MessageCommunicationObserver
        void Initialized() override;
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

        void ServiceDone(Service& service) override;

    protected:
        void RequestSendStream(std::size_t size) override;
        void BusyServiceDone() override;

    private:
        void ProcessMessage();

    private:
        infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
    };
}

#endif
