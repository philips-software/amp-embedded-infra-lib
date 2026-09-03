#include "services/util/EchoOnSesame.hpp"
#include "infra/syntax/ProtoParser.hpp"

namespace services
{
    namespace
    {
        SesameChannel ToSesameChannel(EchoChannel channel)
        {
            switch (channel)
            {
                case EchoChannel::red:
                    return SesameChannel::red;
                case EchoChannel::blue:
                    return SesameChannel::blue;
            }

            std::abort();
        }

        EchoChannel ToEchoChannel(SesameChannel channel)
        {
            switch (channel)
            {
                case SesameChannel::red:
                    return EchoChannel::red;
                case SesameChannel::blue:
                    return EchoChannel::blue;
            }

            std::abort();
        }
    }

    EchoOnSesame::EchoOnSesame(Sesame& subject, services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnStreams(serializerFactory, errorPolicy)
        , SesameObserver(subject)
    {}

    void EchoOnSesame::Reset()
    {
        EchoOnStreams::Reset();
        initialized = false;
        requestedSize.reset();
        requestedChannel = SesameChannel::red;
        sendingProxy = nullptr;

        infra::Subject<EchoInitializationObserver>::NotifyObservers([](auto& observer)
            {
                observer.Reset();
            });

        SesameObserver::Subject().Reset();
    }

    void EchoOnSesame::Initialized()
    {
        EchoOnStreams::Initialized();

        infra::Subject<EchoInitializationObserver>::NotifyObservers([](auto& observer)
            {
                observer.Initialized();
            });

        initialized = true;

        if (requestedSize != std::nullopt)
            RequestSendStream(*std::exchange(requestedSize, std::nullopt));
    }

    void EchoOnSesame::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer, [[maybe_unused]] SesameChannel channel)
    {
        EchoOnStreams::SendStreamAvailable(std::move(writer));
    }

    void EchoOnSesame::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, SesameChannel channel)
    {
        if (ReceiveOnConfiguredChannel(*reader, channel))
            DataReceived(std::move(reader));
    }

    void EchoOnSesame::RequestSendStream(std::size_t size)
    {
        if (initialized)
            SesameObserver::Subject().RequestSendMessage(std::min(size, SesameObserver::Subject().MaxSendMessageSize()), requestedChannel);
        else
            // Before initialization, the maximum window advertised is not yet known, so postpone the RequestSendMessage until initialized
            requestedSize = size;
    }

    void EchoOnSesame::ResetReading()
    {
        SesameObserver::Subject().ResetReading();
        EchoOnStreams::ResetReading();
    }

    void EchoOnSesame::SendingProxySelected(ServiceProxy& proxy)
    {
        sendingProxy = &proxy;
        requestedChannel = ToSesameChannel(proxy.Channel());
    }

    bool EchoOnSesame::ReceiveOnConfiguredChannel(infra::StreamReaderWithRewinding& reader, SesameChannel channel)
    {
        auto marker = reader.ConstructSaveMarker();
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
        infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
        infra::ProtoParser parser(stream, formatErrorPolicy);
        auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
        reader.Rewind(marker);

        if (stream.Failed() || formatErrorPolicy.Failed())
            return true;

        Service* configuredService = nullptr;
        static_cast<services::Echo&>(*this).NotifyObservers([serviceId, &configuredService](auto& service)
            {
                if (service.AcceptsService(serviceId))
                {
                    configuredService = &service;
                    return true;
                }

                return false;
            });

        auto configuredChannel = configuredService == nullptr ? EchoChannel::red : configuredService->Channel();
        return configuredChannel == ToEchoChannel(channel);
    }
}
