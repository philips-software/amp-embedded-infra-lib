#include "services/network/TracingHttpClientImpl.hpp"

namespace services
{
    TracingHttpClientImpl::TracingHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(headerBuffer, hostname)
        , tracer(tracer)
    {}

    void TracingHttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        tracer.Trace() << "HttpClientImpl::SendStreamAvailable; sending request:" << infra::endl;
        request->Write(tracer.Trace());
        HttpClientImpl::SendStreamAvailable(std::move(writer));
    }

    void TracingHttpClientImpl::DataReceived()
    {
        tracer.Trace() << "HttpClientImpl::DataReceived; received response:" << infra::endl;

        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        while (!stream.Empty())
            tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());

        reader = nullptr;

        HttpClientImpl::DataReceived();
    }

    void TracingHttpClientImpl::Connected()
    {
        tracer.Trace() << "HttpClientImpl::Connected";
        HttpClientImpl::Connected();
    }

    void TracingHttpClientImpl::ClosingConnection()
    {
        tracer.Trace() << "HttpClientImpl::ClosingConnection";
        HttpClientImpl::ClosingConnection();
    }
}
