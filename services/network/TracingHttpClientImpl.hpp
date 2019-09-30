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
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<TracingHttpClientImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        TracingHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, Tracer& tracer);

        // Implementation of ConnectionObserver
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;
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
}

#endif
