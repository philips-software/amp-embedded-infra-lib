#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/network/HttpErrors.hpp"
#include "services/network/HttpServer.hpp"
#include <limits>

namespace services
{
    SimpleHttpResponse httpResponseOk{ services::http_responses::ok };

    void HttpPageServer::AddPage(services::HttpPage& page)
    {
        pages.push_front(page);
    }

    services::HttpPage* HttpPageServer::PageForRequest(const HttpRequestParser& request)
    {
        for (auto& page : pages)
            if (page.ServesRequest(request.PathTokens()))
                return &page;

        return nullptr;
    }

    HttpResponseHeaderBuilder::HttpResponseHeaderBuilder(infra::TextOutputStream& output)
        : output(output)
    {
        output << "HTTP/1.1 ";
    }

    HttpResponseHeaderBuilder::HttpResponseHeaderBuilder(infra::TextOutputStream& output, infra::BoundedConstString status)
        : output(output)
    {
        output << "HTTP/1.1 " << status;
    }

    void HttpResponseHeaderBuilder::AddHeader(infra::BoundedConstString key, infra::BoundedConstString value)
    {
        output << "\r\n" << key << ": " << value;
    }

    void HttpResponseHeaderBuilder::AddHeader(infra::BoundedConstString key, uint32_t value)
    {
        output << "\r\n" << key << ": " << value;
    }

    void HttpResponseHeaderBuilder::AddHeader(infra::BoundedConstString key)
    {
        output << "\r\n" << key << ": ";
    }

    void HttpResponseHeaderBuilder::StartBody()
    {
        output << "\r\n\r\n";
    }

    infra::TextOutputStream& HttpResponseHeaderBuilder::Stream()
    {
        return output;
    }

    void HttpResponse::WriteResponse(infra::TextOutputStream& stream) const
    {
        HttpResponseHeaderBuilder builder(stream);
        auto contentType = ContentType();
        std::size_t resultMarker = stream.SaveMarker();
        if (!contentType.empty())
            builder.AddHeader("Content-Length");
        std::size_t sizeMarker = stream.SaveMarker();
        if (!contentType.empty())
            builder.AddHeader("Content-Type", contentType);
        AddHeaders(builder);

        builder.StartBody();
        uint32_t sizeBeforeGetResponse = stream.ProcessedBytesSince(sizeMarker);
        WriteBody(stream);

        if (!contentType.empty())
        {
            uint32_t size = stream.ProcessedBytesSince(sizeMarker) - sizeBeforeGetResponse;
            infra::SavedMarkerTextStream sizeStream(stream, sizeMarker);
            sizeStream << size;
        }

        {
            infra::SavedMarkerTextStream markerStream(stream, resultMarker);
            markerStream << Status();
        }
    }

    infra::BoundedConstString HttpResponse::ContentType() const
    {
        return infra::BoundedConstString();
    }

    void HttpResponse::AddHeaders(HttpResponseHeaderBuilder& builder) const
    {}

    SimpleHttpResponse::SimpleHttpResponse(infra::BoundedConstString status, infra::BoundedConstString body)
        : status(status)
        , body(body)
    {}

    infra::BoundedConstString SimpleHttpResponse::Status() const
    {
        return status;
    }

    void SimpleHttpResponse::WriteBody(infra::TextOutputStream& stream) const
    {
        stream << body;
    }

    HttpServerConnectionObserver::HttpServerConnectionObserver(infra::BoundedString& buffer, HttpPageServer& httpServer)
        : buffer(buffer)
        , httpServer(httpServer)
        , pageLimitedReader([this]() { PageReaderClosed(); })
        , initialIdle(std::chrono::seconds(10), [this]()
            {
                idle = true;
                CheckIdleClose();
            })
    {}

    void HttpServerConnectionObserver::Attached()
    {
        this->connection = &Subject();
        RequestSendStream();
    }

    void HttpServerConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        streamWriter = std::move(writer);

        if (sendingResponse)
        {
            SendBuffer();
            PrepareForNextRequest();
        }
        else
            DataReceived();
    }

    void HttpServerConnectionObserver::DataReceived()
    {
        if (streamWriter == nullptr || pageReader != nullptr)
            return;

        infra::SharedPtr<infra::StreamReaderWithRewinding> reader = Subject().ReceiveStream();
        readerPtr = &reader;

        infra::WeakPtr<ConnectionObserver> weakSelf = Subject().ObserverPtr();

        if (!reader->Empty())
        {
            if (pageServer != nullptr)
                DataReceivedForPage(std::move(reader));
            else if (parser != infra::none)     // Received data after contents for the page, but before closing the page request
                Abort();
            else
                ReceivedRequest(std::move(reader));
        }

        if (weakSelf.lock())
            readerPtr = nullptr;
    }

    void HttpServerConnectionObserver::Detaching()
    {
        if (pageServer != nullptr)
        {
            if (contentLength == infra::none)
                parser->SetContentLength(lengthRead);

            pageServer->Close();
        }

        pageLimitedReader.OnAllocatable([this]() { keepSelfAlive = nullptr; });

        streamWriter = nullptr;

        if (readerPtr != nullptr)
            *readerPtr = nullptr;

        connection = nullptr;
    }

    void HttpServerConnectionObserver::Close()
    {
        closeWhenIdle = true;
        CheckIdleClose();
    }

    void HttpServerConnectionObserver::Abort()
    {
        // TakeOverConnection may have been invoked, which leads to Subject() not being available. However, TakeOverConnection may have been invoked by a page
        // which is about to upgrade to a web connection, but which is not yet upgraded since it is waiting for storage. In that case, if the HttpServer
        // must close down, then we still need to invoke CloseAndDestroy on the connection. Therefore, connection is used instead of Subject()
        if (connection != nullptr)
            connection->AbortAndDestroy();
    }

    void HttpServerConnectionObserver::SendResponse(const HttpResponse& response)
    {
        SendResponseWithoutNextRequest(response);
        PrepareForNextRequest();
    }

    void HttpServerConnectionObserver::SendResponseWithoutNextRequest(const HttpResponse& response)
    {
        buffer.clear();
        infra::StringOutputStream responseStream(buffer);
        response.WriteResponse(responseStream);
        SendingHttpResponse(buffer);

        infra::TextOutputStream::WithErrorPolicy stream(*streamWriter);

        if (send100Response)
            stream << "HTTP/1.1 100 Continue\r\nContent-Length: 0\r\nContent-Type: application/json\r\nStrict-Transport-Security: max-age=31536000\r\n\r\n";

        SendBuffer();
    }

    void HttpServerConnectionObserver::TakeOverConnection(ConnectionObserver& newObserver)
    {
        streamWriter = nullptr;
        if (pageReader != nullptr)
            Subject().AckReceived();
        pageLimitedReader.OnAllocatable(infra::emptyFunction);
        pageServer = nullptr;

        auto& connection = Subject();
        auto newObserverPtr = infra::MakeContainedSharedObject(newObserver, connection.ObserverPtr());
        Detach();
        connection.Attach(newObserverPtr);
    }

    void HttpServerConnectionObserver::SetIdle()
    {
        idle = true;
        CheckIdleClose();
    }

    HttpPage* HttpServerConnectionObserver::PageForRequest(const HttpRequestParser& request)
    {
        auto page = HttpPageServer::PageForRequest(request);
        if (page != nullptr)
            return page;
        else
            return httpServer.PageForRequest(request);
    }

    void HttpServerConnectionObserver::ReceivedTooMuchData(infra::StreamReader& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        while (!stream.Empty())
            stream.ContiguousRange();
        Subject().AckReceived();
        SendResponse(HttpResponseOutOfMemory::Instance());
    }

    void HttpServerConnectionObserver::ReceivedRequest(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(*reader);
        auto start = reader->ConstructSaveMarker();
        auto available = std::min(stream.Available(), buffer.max_size() - buffer.size());
        buffer.resize(buffer.size() + available);
        auto justReceived = buffer.substr(buffer.size() - available);
        stream >> justReceived;

        // First eat up any leftover of previous requests
        auto reducedContentLength = std::min<uint32_t>(contentLength.ValueOr(0), buffer.size());
        buffer.erase(buffer.begin(), buffer.begin() + reducedContentLength);
        if (contentLength != infra::none)
            *contentLength -= reducedContentLength;

        parser.Emplace(buffer);
        if (parser->HeadersComplete())
        {
            reader->Rewind(start + buffer.size());
            Subject().AckReceived();
            TryHandleRequest(std::move(reader));
        }
        else if (!reader->Empty())
            ReceivedTooMuchData(*reader);
        else
            Subject().AckReceived();
    }

    void HttpServerConnectionObserver::TryHandleRequest(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        if (!parser->Valid())
            SendResponse(HttpResponseBadRequest::Instance());
        else
            HandleRequest(std::move(reader));
    }

    void HttpServerConnectionObserver::HandleRequest(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        RequestIsNowInProgress();

        send100Response |= Expect100();

        ServePage(std::move(reader));
    }

    void HttpServerConnectionObserver::RequestIsNowInProgress()
    {
        idle = false;
        initialIdle.Cancel();
        ReceivedHttpRequest(buffer);
    }

    void HttpServerConnectionObserver::ServePage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        contentLength = parser->ContentLength();
        pageServer = PageForRequest(*parser);
        if (pageServer != nullptr)
        {
            infra::WeakPtr<ConnectionObserver> weakSelf = Subject().ObserverPtr();

            pageServer->RequestReceived(*parser, *this);

            if (weakSelf.lock() && pageServer != nullptr)
                DataReceivedForPage(std::move(reader));
        }
        else
            SendResponse(HttpResponseNotFound::Instance());
    }

    void HttpServerConnectionObserver::DataReceivedForPage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        if (contentLength != infra::none && reader->Available() > *contentLength)
            Abort();
        else
        {
            keepSelfAlive = Subject().ObserverPtr();
            pageReader = std::move(reader);
            pageCountingReader.Emplace(*pageReader);
            pageServer->DataReceived(pageLimitedReader.Emplace(*pageCountingReader, contentLength.ValueOr(std::numeric_limits<uint32_t>::max())));
        }
    }

    void HttpServerConnectionObserver::PageReaderClosed()
    {
        if (contentLength != infra::none)
            *contentLength -= pageCountingReader->TotalRead();
        lengthRead += pageCountingReader->TotalRead();
        Subject().AckReceived();
        pageCountingReader = infra::none;
        pageReader = nullptr;

        if (contentLength != infra::none && contentLength == 0)
        {
            pageServer->Close();
            pageServer = nullptr;
        }

        infra::WeakPtr<void> weakSelf = keepSelfAlive;
        keepSelfAlive = nullptr;

        if (weakSelf.lock())
            DataReceived();
    }


    void HttpServerConnectionObserver::RequestSendStream()
    {
        streamWriter = nullptr;
        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    void HttpServerConnectionObserver::PrepareForNextRequest()
    {
        RequestSendStream();
        if (!sendingResponse)
        {
            pageServer = nullptr;
            parser = infra::none;
            lengthRead = 0;
            send100Response = false;
            buffer.clear();
            SetIdle();
        }
    }

    bool HttpServerConnectionObserver::Expect100() const
    {
        return parser->Header("Expect") == "100-continue";
    }

    void HttpServerConnectionObserver::SendBuffer()
    {
        infra::TextOutputStream::WithErrorPolicy stream(*streamWriter);

        auto available = stream.Available();
        stream << buffer.substr(0, available);
        buffer.erase(0, available);
        sendingResponse = !buffer.empty();
    }
    
    void HttpServerConnectionObserver::CheckIdleClose()
    {
        if (closeWhenIdle && idle && IsAttached())
            ConnectionObserver::Close();
    }

    void SimpleHttpPage::RequestReceived(HttpRequestParser& parser, HttpServerConnection& connection)
    {
        this->connection = &connection;
        this->parser = &parser;
    }

    void SimpleHttpPage::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(*reader);
        auto& buffer = parser->BodyBuffer();
        auto available = stream.Available();
        if (available > buffer.max_size() - buffer.size() || buffer.size() + available > parser->ContentLength().ValueOr(std::numeric_limits<uint32_t>::max()))
        {
            while (!stream.Empty())
                stream.ContiguousRange();
            connection->SendResponse(HttpResponseOutOfMemory::Instance());
        }
        else
        {
            auto available = stream.Available();
            buffer.resize(buffer.size() + available);
            auto justReceived = buffer.substr(buffer.size() - available);
            stream >> justReceived;

            reader = nullptr;
        }
    }

    void SimpleHttpPage::Close()
    {
        if (parser->BodyBuffer().size() == parser->ContentLength())
            RespondToRequest(*parser, *connection);
    }

    DefaultHttpServer::DefaultHttpServer(infra::BoundedString& buffer, ConnectionFactory& connectionFactory, uint16_t port)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , buffer(buffer)
        , connectionCreator([this](infra::Optional<HttpServerConnectionObserver>& value, IPAddress address)
            {
                value.Emplace(this->buffer, *this);
            })
    {}
}
