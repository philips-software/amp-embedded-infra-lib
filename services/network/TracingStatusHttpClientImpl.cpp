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

    void TracingStatusHttpClientImpl::ClosingConnection()
    {
        tracer.Trace() << "HttpClientImpl::ClosingConnection";
        HttpClientImpl::ClosingConnection();
    }
}
