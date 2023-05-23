#ifndef PROTOBUF_ECHO_ON_MESSAGE_COMMUNICATION_HPP
#define PROTOBUF_ECHO_ON_MESSAGE_COMMUNICATION_HPP

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
        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

        virtual void ServiceDone(Service& service) override;

    protected:
        virtual void RequestSendStream(std::size_t size) override;
        virtual void BusyServiceDone() override;

    private:
        void ProcessMessage();

    private:
        infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
    };
}

#endif
