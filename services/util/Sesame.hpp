#ifndef SERVICES_MESSAGE_COMMUNICATION_HPP
#define SERVICES_MESSAGE_COMMUNICATION_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/SharedPtr.hpp"

namespace services
{
    class Sesame;
    class SesameEncoded;

    class SesameObserver
        : public infra::SingleObserver<SesameObserver, Sesame>
    {
    public:
        using infra::SingleObserver<SesameObserver, Sesame>::SingleObserver;

        virtual void Initialized() = 0;
        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) = 0;
    };

    class Sesame
        : public infra::Subject<SesameObserver>
    {
    public:
        virtual void RequestSendMessage(std::size_t size) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
    };

    class SesameEncodedObserver
        : public infra::SingleObserver<SesameEncodedObserver, SesameEncoded>
    {
    public:
        using infra::SingleObserver<SesameEncodedObserver, SesameEncoded>::SingleObserver;

        virtual void Initialized() = 0;
        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
        virtual void MessageSent(std::size_t encodedSize) = 0;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, std::size_t encodedSize) = 0;
    };

    class SesameEncoded
        : public infra::Subject<SesameEncodedObserver>
    {
    public:
        virtual void RequestSendMessage(std::size_t size) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
        virtual std::size_t MessageSize(std::size_t size) const = 0;
    };
}

#endif
