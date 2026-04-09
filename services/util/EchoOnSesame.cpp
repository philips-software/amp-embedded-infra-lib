#include "services/util/EchoOnSesame.hpp"

namespace services
{
    EchoOnSesame::EchoOnSesame(Sesame& subject, services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnStreams(serializerFactory, errorPolicy)
        , SesameObserver(subject)
    {}

    void EchoOnSesame::Reset()
    {
        initialized = false;
        requestedSize.reset();
        ReleaseReader();
        SesameObserver::Subject().Reset();
    }

    void EchoOnSesame::Initialized()
    {
        infra::Subject<EchoInitializationObserver>::NotifyObservers([](auto& observer)
            {
                observer.Initialized();
            });

        initialized = true;

        EchoOnStreams::Initialized();

        if (requestedSize != std::nullopt)
            RequestSendStream(*std::exchange(requestedSize, std::nullopt));
    }

    void EchoOnSesame::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        EchoOnStreams::SendStreamAvailable(std::move(writer));
    }

    void EchoOnSesame::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        DataReceived(std::move(reader));
    }

    void EchoOnSesame::RequestSendStream(std::size_t size)
    {
        if (initialized)
            SesameObserver::Subject().RequestSendMessage(std::min(size, SesameObserver::Subject().MaxSendMessageSize()));
        else
            // Before initialization, the maximum window advertised is not yet known, so postpone the RequestSendMessage until initialized
            requestedSize = size;
    }
}
