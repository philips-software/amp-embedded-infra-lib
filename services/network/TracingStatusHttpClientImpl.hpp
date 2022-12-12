#ifndef SERVICES_TRACING_STATUS_HTTP_CLIENT_IMPL_HPP
#define SERVICES_TRACING_STATUS_HTTP_CLIENT_IMPL_HPP

#include "services/network/HttpClientImpl.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingStatusHttpClientImpl
        : public HttpClientImpl
    {
    public:
        TracingStatusHttpClientImpl(infra::BoundedConstString hostname, Tracer& tracer);

    protected:
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;
        virtual void Detaching() override;

    private:
        friend class TracingStatusHttpClientConnectorImpl;

        Tracer& tracer;
    };

    class TracingStatusHttpClientImplWithRedirection
        : public HttpClientImplWithRedirection
    {
    public:
        template<std::size_t MaxSize>
        using WithRedirectionUrlSize = infra::WithStorage<TracingStatusHttpClientImplWithRedirection, infra::BoundedString::WithStorage<MaxSize>>;

        TracingStatusHttpClientImplWithRedirection(infra::BoundedString redirectedUrlStorage, infra::BoundedConstString hostname, ConnectionFactoryWithNameResolver& connectionFactory, Tracer& tracer);

    protected:
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;
        virtual void Detaching() override;
        virtual void Redirecting(infra::BoundedConstString url) override;

    private:
        friend class TracingStatusHttpClientConnectorImpl;

        Tracer& tracer;
    };
}

#endif
