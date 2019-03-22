#ifndef SERVICES_TRACING_HTTP_CLIENT_IMPL_HPP
#define SERVICES_TRACING_HTTP_CLIENT_IMPL_HPP

#include "services/network/HttpClientImpl.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingHttpClientImpl
        : public HttpClientImpl
    {
    public:
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<TracingHttpClientImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        TracingHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, Tracer& tracer);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;

    private:
        friend class TracingHttpClientConnectorImpl;
        
        Tracer& tracer;
    };

    class TracingHttpClientConnectorImpl
        : public HttpClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<TracingHttpClientConnectorImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        TracingHttpClientConnectorImpl(infra::BoundedString& headerBuffer, ConnectionFactoryWithNameResolver& connectionFactory, Tracer& tracer);

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

        // Implementation of HttpClientConnector
        virtual void Connect(HttpClientObserverFactory& factory) override;
        virtual void CancelConnect(HttpClientObserverFactory& factory) override;

    private:
        void TryConnectWaiting();

    private:
        infra::BoundedString& headerBuffer;
        ConnectionFactoryWithNameResolver& connectionFactory;
        infra::NotifyingSharedOptional<TracingHttpClientImpl> client;
        Tracer& tracer;

        HttpClientObserverFactory* clientObserverFactory = nullptr;
        infra::IntrusiveList<HttpClientObserverFactory> waitingClientObserverFactories;
    };
}

#endif
