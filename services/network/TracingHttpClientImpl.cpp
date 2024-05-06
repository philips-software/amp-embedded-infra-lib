#include "services/network/TracingHttpClientImpl.hpp"

namespace services
{
    TracingHttpClientImpl::TracingHttpClientImpl(infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(hostname)
        , tracer(tracer)
    {}

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

    void TracingHttpClientImpl::Attached()
    {
        tracer.Trace() << "HttpClientImpl::Attached";
        HttpClientImpl::Attached();
    }

    void TracingHttpClientImpl::Detaching()
    {
        tracer.Trace() << "HttpClientImpl::Detaching";
        HttpClientImpl::Detaching();
    }

    void TracingHttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        auto tracingWriterPtr = tracingWriter.emplace(std::move(writer), tracer);
        HttpClientImpl::SendStreamAvailable(infra::MakeContainedSharedObject(tracingWriterPtr->Writer(), std::move(tracingWriterPtr)));
    }

    TracingHttpClientImpl::TracingWriter::TracingWriter(infra::SharedPtr<infra::StreamWriter>&& writer, services::Tracer& tracer)
        : writer(std::move(writer))
        , tracingWriter(*this->writer, tracer)
    {}

    infra::StreamWriter& TracingHttpClientImpl::TracingWriter::Writer()
    {
        return tracingWriter;
    }

    TracingHttpClientImplWithRedirection::TracingHttpClientImplWithRedirection(infra::BoundedString redirectedUrlStorage, infra::BoundedConstString hostname, ConnectionFactoryWithNameResolver& connectionFactory, services::Tracer& tracer)
        : HttpClientImplWithRedirection(redirectedUrlStorage, hostname, connectionFactory)
        , tracer(tracer)
    {}

    void TracingHttpClientImplWithRedirection::DataReceived()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::DataReceived; received response:" << infra::endl;

        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        while (!stream.Empty())
            tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());

        reader = nullptr;

        HttpClientImplWithRedirection::DataReceived();
    }

    void TracingHttpClientImplWithRedirection::Attached()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::Attached";
        HttpClientImplWithRedirection::Attached();
    }

    void TracingHttpClientImplWithRedirection::Detaching()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::Detaching";
        HttpClientImplWithRedirection::Detaching();
    }

    void TracingHttpClientImplWithRedirection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        auto tracingWriterPtr = tracingWriter.emplace(std::move(writer), tracer);
        HttpClientImplWithRedirection::SendStreamAvailable(infra::MakeContainedSharedObject(tracingWriterPtr->Writer(), std::move(tracingWriterPtr)));
    }

    void TracingHttpClientImplWithRedirection::Redirecting(infra::BoundedConstString url)
    {
        tracer.Trace() << "HttpClientImplWithRedirection redirecting to " << url;
        HttpClientImplWithRedirection::Redirecting(url);
    }

    TracingHttpClientImplWithRedirection::TracingWriter::TracingWriter(infra::SharedPtr<infra::StreamWriter>&& writer, services::Tracer& tracer)
        : writer(std::move(writer))
        , tracingWriter(*this->writer, tracer)
    {}

    infra::StreamWriter& TracingHttpClientImplWithRedirection::TracingWriter::Writer()
    {
        return tracingWriter;
    }
}
