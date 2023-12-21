#ifndef SERVICES_MESSAGE_COMMUNICATION_HPP
#define SERVICES_MESSAGE_COMMUNICATION_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/SharedPtr.hpp"

namespace services
{
    class MessageCommunication;

    class MessageCommunicationObserver
        : public infra::SingleObserver<MessageCommunicationObserver, MessageCommunication>
    {
    public:
        using infra::SingleObserver<MessageCommunicationObserver, MessageCommunication>::SingleObserver;

        virtual void Initialized() = 0;
        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) = 0;
    };

    class MessageCommunication
        : public infra::Subject<MessageCommunicationObserver>
    {
    public:
        virtual void RequestSendMessage(uint16_t size) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
    };
}

#endif
