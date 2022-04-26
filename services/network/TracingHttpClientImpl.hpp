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
        virtual void Attached() override;
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

    class TracingHttpClientImplWithRedirection
        : public HttpClientImplWithRedirection
    {
    public:
        template<std::size_t MaxSize>
        using WithRedirectionUrlSize = infra::WithStorage<TracingHttpClientImplWithRedirection, infra::BoundedString::WithStorage<MaxSize>>;

        TracingHttpClientImplWithRedirection(infra::BoundedString redirectedUrlStorage, infra::BoundedConstString hostname, ConnectionFactoryWithNameResolver& connectionFactory, Tracer& tracer);

        // Implementation of ConnectionObserver
        virtual void DataReceived() override;
        virtual void Attached() override;
        virtual void Detaching() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        // Implementation of HttpClientImplWithRedirection
        virtual void Redirecting(infra::BoundedConstString url) override;

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
        TracingHttpClientConnectorImpl(services::ConnectionFactory& connectionFactory, services::IPAddress address, Args&&... args, services::Tracer& tracer);

        // Implementation of ClientConnectionObserverFactory
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(typename HttpClientConnectorImpl<HttpClient, Args...>::ConnectFailReason reason) override;

    private:
        services::Tracer& tracer;
    };

    template<class HttpClient = HttpClientImpl, class... Args>
    class TracingHttpClientConnectorWithNameResolverImpl
        : public HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>
    {
    public:
        TracingHttpClientConnectorWithNameResolverImpl(ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args, services::Tracer& tracer);

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(typename HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectFailReason reason) override;

    private:
        services::Tracer& tracer;
    };

    //// Implementation ////

    template<class HttpClient, class... Args>
    TracingHttpClientConnectorImpl<HttpClient, Args...>::TracingHttpClientConnectorImpl(services::ConnectionFactory& connectionFactory, services::IPAddress address, Args&&... args, services::Tracer& tracer)
        : HttpClientConnectorImpl<HttpClient, Args...>(connectionFactory, address, std::forward<Args>(args)...)
        , tracer(tracer)
    {}

    template<class HttpClient, class... Args>
    void TracingHttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        tracer.Trace() << "HttpClientConnectorImpl::ConnectionEstablished with " << this->Address();
        HttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(std::move(createdObserver));
    }

    template<class HttpClient, class... Args>
    void TracingHttpClientConnectorImpl<HttpClient, Args...>::ConnectionFailed(typename HttpClientConnectorImpl<HttpClient, Args...>::ConnectFailReason reason)
    {
        tracer.Trace() << "HttpClientConnectorImpl::ConnectionFailed with " << this->Address() << " reason: " << reason;
        HttpClientConnectorImpl<HttpClient, Args...>::ConnectionFailed(reason);
    }

    template<class HttpClient, class... Args>
    TracingHttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::TracingHttpClientConnectorWithNameResolverImpl(ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args, services::Tracer& tracer)
        : HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>(connectionFactory, std::forward<Args>(args)...)
        , tracer(tracer)
    {}

    template<class HttpClient, class... Args>
    void TracingHttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        tracer.Trace() << "HttpClientConnectorWithNameResolverImpl::ConnectionEstablished with " << this->Hostname();
        HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionEstablished(std::move(createdObserver));
    }

    template<class HttpClient, class... Args>
    void TracingHttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionFailed(typename HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectFailReason reason)
    {
        tracer.Trace() << "HttpClientConnectorWithNameResolverImpl::ConnectionFailed with " << this->Hostname() << " reason: " << reason;
        HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionFailed(reason);
    }
}

#endif
