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

    class MessageCommunicationReceiveOnInterrupt;

    class MessageCommunicationReceiveOnInterruptObserver
        : public infra::SingleObserver<MessageCommunicationReceiveOnInterruptObserver, MessageCommunicationReceiveOnInterrupt>
    {
    public:
        using infra::SingleObserver<MessageCommunicationReceiveOnInterruptObserver, MessageCommunicationReceiveOnInterrupt>::SingleObserver;

        virtual void ReceivedMessageOnInterrupt(infra::StreamReader& reader) = 0;
    };

    class MessageCommunicationReceiveOnInterrupt
        : public infra::Subject<MessageCommunicationReceiveOnInterruptObserver>
    {
    public:
        virtual infra::SharedPtr<infra::StreamWriter> SendMessageStream(uint16_t size, const infra::Function<void(uint16_t size)>& onSent) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
    };
}

#endif
