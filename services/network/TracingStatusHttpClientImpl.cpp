#include "services/network/TracingStatusHttpClientImpl.hpp"

namespace services
{
    TracingStatusHttpClientImpl::TracingStatusHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(headerBuffer, hostname)
        , tracer(tracer)
    {}

    void TracingStatusHttpClientImpl::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        tracer.Trace() << "HttpClientImpl::StatusAvailable " << statusLine;
        HttpClientImpl::StatusAvailable(code, statusLine);
    }
}
