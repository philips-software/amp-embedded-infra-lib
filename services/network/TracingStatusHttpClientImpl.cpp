#include "services/network/TracingStatusHttpClientImpl.hpp"

namespace services
{
    TracingStatusHttpClientImpl::TracingStatusHttpClientImpl(infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(hostname)
        , tracer(tracer)
    {}

    void TracingStatusHttpClientImpl::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        tracer.Trace() << "HttpClientImpl::StatusAvailable " << statusLine;
        HttpClientImpl::StatusAvailable(code, statusLine);
    }

    void TracingStatusHttpClientImpl::Detaching()
    {
        tracer.Trace() << "HttpClientImpl::Detaching";
        HttpClientImpl::Detaching();
    }

    TracingStatusHttpClientImplWithRedirection::TracingStatusHttpClientImplWithRedirection(infra::BoundedString redirectedUrlStorage, infra::BoundedConstString hostname, ConnectionFactoryWithNameResolver& connectionFactory, services::Tracer& tracer)
        : HttpClientImplWithRedirection(redirectedUrlStorage, hostname, connectionFactory)
        , tracer(tracer)
    {}

    void TracingStatusHttpClientImplWithRedirection::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        tracer.Trace() << "HttpClientImplWithRedirection::StatusAvailable " << statusLine;
        HttpClientImplWithRedirection::StatusAvailable(code, statusLine);
    }

    void TracingStatusHttpClientImplWithRedirection::Detaching()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::Detaching";
        HttpClientImplWithRedirection::Detaching();
    }

    void TracingStatusHttpClientImplWithRedirection::Redirecting(infra::BoundedConstString url)
    {
        tracer.Trace() << "HttpClientImplWithRedirection redirecting to " << url;
        HttpClientImplWithRedirection::Redirecting(url);
    }
}
