#include "services/network/TracingHttpClientImpl.hpp"

namespace services
{
    TracingHttpClientImpl::TracingHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(headerBuffer, hostname)
        , tracer(tracer)
    {}

    void TracingHttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        tracer.Trace() << "HttpClientImpl::SendStreamAvailable; sending request:" << infra::endl;
        request->Write(tracer.Trace());
        HttpClientImpl::SendStreamAvailable(std::move(writer));
    }

    void TracingHttpClientImpl::DataReceived()
    {
        tracer.Trace() << "HttpClientImpl::DataReceived; received response:" << infra::endl;

        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        while (!stream.Empty())
            tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());

        reader = nullptr;

        HttpClientImpl::DataReceived();
    }

    void TracingHttpClientImpl::Connected()
    {
        tracer.Trace() << "HttpClientImpl::Connected";
        HttpClientImpl::Connected();
    }

    void TracingHttpClientImpl::ClosingConnection()
    {
        tracer.Trace() << "HttpClientImpl::ClosingConnection";
        HttpClientImpl::ClosingConnection();
    }

    TracingHttpClientConnectorImpl::TracingHttpClientConnectorImpl(infra::BoundedString& headerBuffer, services::ConnectionFactoryWithNameResolver& connectionFactory, Tracer& tracer)
        : headerBuffer(headerBuffer)
        , connectionFactory(connectionFactory)
        , client([this]() { TryConnectWaiting(); })
        , tracer(tracer)
    {}

    infra::BoundedConstString TracingHttpClientConnectorImpl::Hostname() const
    {
        return clientObserverFactory->Hostname();
    }

    uint16_t TracingHttpClientConnectorImpl::Port() const
    {
        return clientObserverFactory->Port();
    }

    void TracingHttpClientConnectorImpl::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory != nullptr);
        auto clientPtr = client.Emplace(headerBuffer, Hostname(), tracer);

        clientObserverFactory->ConnectionEstablished([&clientPtr, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
        {
            if (observer)
            {
                clientPtr->observer = observer;
                observer->Attach(*clientPtr);
                createdObserver(clientPtr);
            }
        });

        clientObserverFactory = nullptr;
    }

    void TracingHttpClientConnectorImpl::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientObserverFactory != nullptr);

        switch (reason)
        {
            case ConnectFailReason::refused:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::refused);
                break;
            case ConnectFailReason::connectionAllocationFailed:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                break;
            case ConnectFailReason::nameLookupFailed:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
                break;
            default:
                std::abort();
        }

        clientObserverFactory = nullptr;
    }

    void TracingHttpClientConnectorImpl::Connect(HttpClientObserverFactory& factory)
    {
        waitingClientObserverFactories.push_back(factory);
        TryConnectWaiting();
    }

    void TracingHttpClientConnectorImpl::CancelConnect(HttpClientObserverFactory& factory)
    {
        if (clientObserverFactory == &factory)
        {
            connectionFactory.CancelConnect(*this);
            clientObserverFactory = nullptr;
        }
        else
            waitingClientObserverFactories.erase(factory);

        TryConnectWaiting();
    }

    void TracingHttpClientConnectorImpl::TryConnectWaiting()
    {
        if (client.Allocatable() && !waitingClientObserverFactories.empty())
        {
            clientObserverFactory = &waitingClientObserverFactories.front();
            waitingClientObserverFactories.pop_front();
            connectionFactory.Connect(*this);
        }
    }
}
