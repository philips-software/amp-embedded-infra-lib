#include "services/network/TracingStatusHttpClientImpl.hpp"
#include <algorithm>

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

    void TracingStatusHttpClientImpl::DataReceived()
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

    void TracingStatusHttpClientImpl::HeaderParsingDone(bool error)
    {
        if (error)
            tracer.Trace() << "HttpClientImpl::HeaderParsingDone - ERROR: Header parsing failed";
        HttpClientImpl::HeaderParsingDone(error);
    }

    void TracingStatusHttpClientImpl::OnAbortingConnection()
    {
        tracer.Trace() << "HttpClientImpl::OnAbortingConnection - Aborting due to: "
                       << (HeaderParsingError() ? "header parsing error" : "missing Content-Length");
        HttpClientImpl::OnAbortingConnection();
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

    void TracingStatusHttpClientImplWithRedirection::DataReceived()
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

    void TracingStatusHttpClientImplWithRedirection::HeaderParsingDone(bool error)
    {
        if (error)
            tracer.Trace() << "HttpClientImplWithRedirection::HeaderParsingDone - ERROR: Header parsing failed";
        HttpClientImplWithRedirection::HeaderParsingDone(error);
    }

    void TracingStatusHttpClientImplWithRedirection::OnAbortingConnection()
    {
        tracer.Trace() << "HttpClientImplWithRedirection::OnAbortingConnection - Aborting due to: "
                       << (HeaderParsingError() ? "header parsing error" : "missing Content-Length");
        HttpClientImplWithRedirection::OnAbortingConnection();
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
