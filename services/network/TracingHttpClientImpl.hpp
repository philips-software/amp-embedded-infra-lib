#ifndef SERVICES_TRACING_HTTP_CLIENT_IMPL_HPP
#define SERVICES_TRACING_HTTP_CLIENT_IMPL_HPP

#include "services/network/HttpClientImpl.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/tracer/TracingOutputStream.hpp"

namespace services
{
    class TracingHttpClientImpl
        : public HttpClientImpl
    {
    public:
        TracingHttpClientImpl(infra::BoundedConstString hostname, Tracer& tracer);

        // Implementation of ConnectionObserver
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void Detaching() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        class TracingWriter
        {
        public:
            TracingWriter(infra::SharedPtr<infra::StreamWriter>&& writer, services::Tracer& tracer);

            infra::StreamWriter& Writer();

        private:
            infra::SharedPtr<infra::StreamWriter> writer;
            TracingStreamWriter tracingWriter;
        };

    private:
        Tracer& tracer;

        infra::SharedOptional<TracingWriter> tracingWriter;
    };

    template<class HttpClient = HttpClientImpl, class... Args>
    class TracingHttpClientConnectorImpl
        : public HttpClientConnectorImpl<HttpClient, Args...>
    {
    public:
        TracingHttpClientConnectorImpl(ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args, services::Tracer& tracer);

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(typename HttpClientConnectorImpl<HttpClient, Args...>::ConnectFailReason reason) override;

    private:
        services::Tracer& tracer;
    };

    //// Implementation ////

    template<class HttpClient, class... Args>
    TracingHttpClientConnectorImpl<HttpClient, Args...>::TracingHttpClientConnectorImpl(ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args, services::Tracer& tracer)
        : HttpClientConnectorImpl<HttpClient, Args...>(connectionFactory, std::forward<Args>(args)...)
        , tracer(tracer)
    {}

    template<class HttpClient, class... Args>
    void TracingHttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        tracer.Trace() << "HttpClientConnectorImpl::ConnectionEstablished with " << this->Hostname();
        HttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(std::move(createdObserver));
    }

    template<class HttpClient, class... Args>
    void TracingHttpClientConnectorImpl<HttpClient, Args...>::ConnectionFailed(typename HttpClientConnectorImpl<HttpClient, Args...>::ConnectFailReason reason)
    {
        tracer.Trace() << "HttpClientConnectorImpl::ConnectionFailed with " << this->Hostname() << " reason: " << reason;
        HttpClientConnectorImpl<HttpClient, Args...>::ConnectionFailed(reason);
    }
}

#endif
