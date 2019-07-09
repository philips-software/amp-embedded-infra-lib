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

        // Implementation of HttpClientImpl
        virtual void WriteRequest(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        // Implementation of ConnectionObserver
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;

    private:
        friend class TracingHttpClientConnectorImpl;
        
        Tracer& tracer;
    };
}

#endif
