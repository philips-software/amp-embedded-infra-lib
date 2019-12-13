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
        virtual void ClosingConnection() override;

    private:
        friend class TracingStatusHttpClientConnectorImpl;
        
        Tracer& tracer;
    };
}

#endif
