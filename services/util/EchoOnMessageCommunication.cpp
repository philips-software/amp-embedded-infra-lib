#include "services/util/EchoOnMessageCommunication.hpp"

namespace services
{
    EchoOnMessageCommunication::EchoOnMessageCommunication(MessageCommunication& subject, EchoErrorPolicy& errorPolicy)
        : EchoOnStreams(errorPolicy)
        , MessageCommunicationObserver(subject)
    {}

    void EchoOnMessageCommunication::Initialized()
    {}

    void EchoOnMessageCommunication::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        SetStreamWriter(std::move(writer));
    }

    void EchoOnMessageCommunication::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        this->reader = std::move(reader);
        ProcessMessage();
    }

    void EchoOnMessageCommunication::ServiceDone(Service& service)
    {
        reader = nullptr;
        EchoOnStreams::ServiceDone(service);
    }

    void EchoOnMessageCommunication::RequestSendStream(std::size_t size)
    {
        MessageCommunicationObserver::Subject().RequestSendMessage(static_cast<uint16_t>(size));
    }

    void EchoOnMessageCommunication::BusyServiceDone()
    {
        // In this class, services are never busy, so BusyServiceDone() is never invoked
        std::abort();
    }

    void EchoOnMessageCommunication::ProcessMessage()
    {
        if (!EchoOnStreams::ProcessMessage(*reader))
            errorPolicy.MessageFormatError();
    }
}
