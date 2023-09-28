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
        EchoOnStreams::SendStreamAvailable(std::move(writer));
    }

    void EchoOnMessageCommunication::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        DataReceived(std::move(reader));
    }

    void EchoOnMessageCommunication::RequestSendStream(std::size_t size)
    {
        MessageCommunicationObserver::Subject().RequestSendMessage(static_cast<uint16_t>(std::min(size, MessageCommunicationObserver::Subject().MaxSendMessageSize())));
    }
}
