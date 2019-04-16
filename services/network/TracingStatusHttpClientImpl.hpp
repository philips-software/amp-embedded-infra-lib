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
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<TracingStatusHttpClientImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        TracingStatusHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, Tracer& tracer);

    protected:
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;

    private:
        friend class TracingStatusHttpClientConnectorImpl;
        
        Tracer& tracer;
    };
}

#endif
