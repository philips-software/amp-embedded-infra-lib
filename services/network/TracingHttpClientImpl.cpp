#include "services/network/TracingHttpClientImpl.hpp"
#include <algorithm>

namespace services
{
    TracingHttpClientImpl::TracingHttpClientImpl(infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(hostname)
        , tracer(tracer)
    {}

    void TracingHttpClientImpl::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        tracer.Trace() << "HttpClientImplWithRedirection::StatusAvailable: " << code << " " << statusLine;
        HttpClientImpl::StatusAvailable(code, statusLine);
    }

    void TracingHttpClientImpl::HeaderParsingDone(bool error)
    {
        if (error)
            tracer.Trace() << "HttpClientImpl::HeaderParsingDone - ERROR: Header parsing failed (invalid status line, unparseable status code, or header buffer overflow)";
        else
            tracer.Trace() << "HttpClientImpl::HeaderParsingDone - Headers parsed successfully";
        HttpClientImpl::HeaderParsingDone(error);
    }

    void TracingHttpClientImpl::OnAbortingConnection()
    {
        tracer.Trace() << "HttpClientImpl::OnAbortingConnection - Aborting connection due to: "
                       << (HeaderParsingError() ? "header parsing error" : "missing Content-Length header");
        HttpClientImpl::OnAbortingConnection();
    }

    void TracingHttpClientImpl::BodyReaderAvailable(infra::SharedPtr<infra::CountingStreamReaderWithRewinding>&& bodyReader)
    {
        tracer.Trace() << "HttpClientImpl::BodyAvailable; received response:" << infra::endl;

        auto reader = bodyReader;
        infra::DataInputStream::WithErrorPolicy stream(*reader);
        auto marker = reader->ConstructSaveMarker();
        while (!stream.Empty())
            tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());

        reader->Rewind(marker);
        HttpClientImpl::BodyReaderAvailable(std::move(reader));
    }

    void TracingHttpClientImpl::Attached()
    {
        tracer.Trace() << "HttpClientImpl::Attached";
        HttpClientImpl::Attached();
    }

    void TracingHttpClientImpl::DataReceived()
    {
        auto receiveStream = GetConnection().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*receiveStream);

        if (!stream.Empty())
        {
            tracer.Trace() << "HttpClientImpl::DataReceived - Received " << stream.Available() << " bytes:";
            auto marker = receiveStream->ConstructSaveMarker();

            // Limit output to first 512 bytes to avoid log overflow
            std::size_t bytesToLog = std::min(stream.Available(), static_cast<std::size_t>(512));
            infra::LimitedStreamReader limitedReader(*receiveStream, bytesToLog);
            infra::DataInputStream::WithErrorPolicy limitedStream(limitedReader);

            while (!limitedStream.Empty())
                tracer.Trace() << infra::ByteRangeAsString(limitedStream.ContiguousRange());

            if (stream.Available() > 512)
                tracer.Trace() << "... (" << (stream.Available() - 512) << " more bytes not shown)";

            receiveStream->Rewind(marker);
        }

        HttpClientImpl::DataReceived();
    }

    void TracingHttpClientImpl::Detaching()
    {
        tracer.Trace() << "HttpClientImpl::Detaching";
        HttpClientImpl::Detaching();
    }

    void TracingHttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        auto tracingWriterPtr = tracingWriter.Emplace(std::move(writer), tracer);
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

    void TracingHttpClientImplWithRedirection::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        tracer.Trace() << "HttpClientImplWithRedirection::StatusAvailable: " << code << " " << statusLine;
        HttpClientImplWithRedirection::StatusAvailable(code, statusLine);
    }

    void TracingHttpClientImplWithRedirection::HeaderParsingDone(bool error)
    {
        if (error)
            tracer.Trace() << "HttpClientImplWithRedirection::HeaderParsingDone - ERROR: Header parsing failed (invalid status line, unparseable status code, or header buffer overflow)";
        else
            tracer.Trace() << "HttpClientImplWithRedirection::HeaderParsingDone - Headers parsed successfully";
        HttpClientImplWithRedirection::HeaderParsingDone(error);
    }

    void TracingHttpClientImplWithRedirection::OnAbortingConnection()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::OnAbortingConnection - Aborting connection due to: "
                       << (HeaderParsingError() ? "header parsing error" : "missing Content-Length header");
        HttpClientImplWithRedirection::OnAbortingConnection();
    }

    void TracingHttpClientImplWithRedirection::BodyReaderAvailable(infra::SharedPtr<infra::CountingStreamReaderWithRewinding>&& bodyReader)
    {
        tracer.Trace() << "HttpClientImplWithRedirection::BodyAvailable; received response:" << infra::endl;

        auto reader = bodyReader;
        infra::DataInputStream::WithErrorPolicy stream(*reader);
        auto marker = reader->ConstructSaveMarker();
        while (!stream.Empty())
            tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());

        reader->Rewind(marker);
        HttpClientImplWithRedirection::BodyReaderAvailable(std::move(reader));
    }

    void TracingHttpClientImplWithRedirection::Attached()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::Attached";
        HttpClientImplWithRedirection::Attached();
    }

    void TracingHttpClientImplWithRedirection::DataReceived()
    {
        auto receiveStream = GetConnection().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*receiveStream);

        if (!stream.Empty())
        {
            tracer.Trace() << "HttpClientImplWithRedirection::DataReceived - Received " << stream.Available() << " bytes:";
            auto marker = receiveStream->ConstructSaveMarker();

            // Limit output to first 512 bytes to avoid log overflow
            std::size_t bytesToLog = std::min(stream.Available(), static_cast<std::size_t>(512));
            infra::LimitedStreamReader limitedReader(*receiveStream, bytesToLog);
            infra::DataInputStream::WithErrorPolicy limitedStream(limitedReader);

            while (!limitedStream.Empty())
                tracer.Trace() << infra::ByteRangeAsString(limitedStream.ContiguousRange());

            if (stream.Available() > 512)
                tracer.Trace() << "... (" << (stream.Available() - 512) << " more bytes not shown)";

            receiveStream->Rewind(marker);
        }

        HttpClientImplWithRedirection::DataReceived();
    }

    void TracingHttpClientImplWithRedirection::Detaching()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::Detaching";
        HttpClientImplWithRedirection::Detaching();
    }

    void TracingHttpClientImplWithRedirection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        auto tracingWriterPtr = tracingWriter.Emplace(std::move(writer), tracer);
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
