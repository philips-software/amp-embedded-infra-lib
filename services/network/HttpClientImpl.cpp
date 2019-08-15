#include "services/network/HttpClientImpl.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    namespace
    {
        // Naming is according to rfc7230
        const infra::BoundedConstString httpVersion = "HTTP/1.1";
        const infra::BoundedConstString sp = " ";
        const infra::BoundedConstString crlf = "\r\n";
    }

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, const HttpHeaders headers)
        : HttpRequestFormatter(verb, hostname, requestTarget, infra::BoundedConstString(), headers)
    {}

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers)
        : verb(verb)
        , requestTarget(requestTarget)
        , content(content)
        , hostHeader("host", hostname)
        , headers(headers)
    {
        if (!content.empty())
            AddContentLength(content.size());
    }

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers)
        : verb(verb)
        , requestTarget(requestTarget)
        , hostHeader("host", hostname)
        , headers(headers)
    {
        AddContentLength(contentSize);
    }

    std::size_t HttpRequestFormatter::Size() const
    {
        return HttpVerbToString(verb).size() + requestTarget.size() + httpVersion.size() + HeadersSize() + (2 * crlf.size()) + (2 * sp.size()) + content.size();
    }

    void HttpRequestFormatter::Write(infra::TextOutputStream stream) const
    {
        stream << verb << sp << requestTarget << sp << httpVersion << crlf;

        for (auto&& header : headers)
            stream << header << crlf;

        stream << hostHeader << crlf;

        if (contentLengthHeader)
            stream << *contentLengthHeader << crlf;

        stream << crlf;
        stream << content;
    }

    void HttpRequestFormatter::AddContentLength(std::size_t size)
    {
        infra::StringOutputStream contentLengthStream(contentLength);
        contentLengthStream << static_cast<uint64_t>(size);
        contentLengthHeader.Emplace("content-length", contentLength);
    }

    std::size_t HttpRequestFormatter::HeadersSize() const
    {
        std::size_t headerSize = 0;
        for (auto&& header : headers)
            headerSize += (header.Size() + crlf.size());

        if (contentLengthHeader)
            headerSize += contentLengthHeader->Size() + crlf.size();

        headerSize += hostHeader.Size() + crlf.size();

        return headerSize;
    }

    HttpClientImpl::HttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname)
        : headerBuffer(headerBuffer)
        , hostname(hostname)
    {}

    void HttpClientImpl::AttachObserver(const infra::SharedPtr<HttpClientObserver>& observer)
    {
        this->observer = observer;
        observer->Attach(*this);
    }

    void HttpClientImpl::Get(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest(HttpVerb::get, requestTarget, headers);
    }

    void HttpClientImpl::Head(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest(HttpVerb::head, requestTarget, headers);
    }

    void HttpClientImpl::Connect(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest(HttpVerb::connect, requestTarget, headers);
    }

    void HttpClientImpl::Options(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest(HttpVerb::options, requestTarget, headers);
    }

    void HttpClientImpl::Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::post, requestTarget, content, headers);
    }

    void HttpClientImpl::Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::post, requestTarget, contentSize, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, content, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, contentSize, headers);
    }

    void HttpClientImpl::Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::patch, requestTarget, content, headers);
    }

    void HttpClientImpl::Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::delete_, requestTarget, content, headers);
    }

    void HttpClientImpl::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    void HttpClientImpl::Close()
    {
        bodyReaderAccess.SetAction([]() {});
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void HttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (forwardingStream)
            ForwardStream(std::move(writer));
        else
            WriteRequest(std::move(writer));
    }

    void HttpClientImpl::DataReceived()
    {
        if (bodyReader != infra::none)
            observer->BodyAvailable(infra::MakeContainedSharedObject(bodyReader->countingReader, bodyReaderAccess.MakeShared(bodyReader)));
        else
        {
            if (response)
                HandleData();
            else
                AbortAndDestroy();
        }
    }

    void HttpClientImpl::Connected()
    {
        infra::WeakPtr<services::HttpClientImpl> self = infra::StaticPointerCast<HttpClientImpl>(services::ConnectionObserver::Subject().Observer());
        bodyReaderAccess.SetAction([self]()
        {
            if (auto sharedSelf = self.lock())
                sharedSelf->BodyReaderDestroyed();
        });

        GetObserver().Connected();
    }

    void HttpClientImpl::ClosingConnection()
    {
        GetObserver().ClosingConnection();
        observer->Detach();
    }

    void HttpClientImpl::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        observer->StatusAvailable(code);
    }

    void HttpClientImpl::WriteRequest(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        request->Write(stream);
        request = infra::none;
        writer = nullptr;

        RequestForwardStreamOrExpectResponse();
    }

    void HttpClientImpl::ForwardStream(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        forwardStreamPtr = std::move(writer);
        auto available = forwardStreamPtr->Available();
        forwardStreamAccess.SetAction([this, available]()
        {
            streamingContentSize -= available - forwardStreamPtr->Available();

            forwardStreamPtr = nullptr;

            RequestForwardStreamOrExpectResponse();
        });

        observer->SendStreamAvailable(forwardStreamAccess.MakeShared(*forwardStreamPtr));
    }

    void HttpClientImpl::RequestForwardStreamOrExpectResponse()
    {
        forwardingStream = streamingContentSize != 0;

        if (forwardingStream)
            ConnectionObserver::Subject().RequestSendStream(std::min(streamingContentSize, ConnectionObserver::Subject().MaxSendStreamSize()));
        else
            response.Emplace(*this, headerBuffer);
    }

    void HttpClientImpl::HandleData()
    {
        if (!response->Done())
        {
            auto reader = ConnectionObserver::Subject().ReceiveStream();

            infra::WeakPtr<services::ConnectionObserver> self = services::ConnectionObserver::Subject().Observer();

            response->DataReceived(*reader);

            if (!self.lock())   // DataReceived may close the connection
                return;

            ConnectionObserver::Subject().AckReceived();
        }

        if (response->Done())
        {
            if (!response->Error())
                BodyReceived();
            else
                AbortAndDestroy();
        }
    }

    void HttpClientImpl::BodyReceived()
    {
        if (!contentLength)
            contentLength = response->ContentLength();

        if (contentLength == 0)
            BodyComplete();
        else
        {
            bodyReader.Emplace(ConnectionObserver::Subject().ReceiveStream(), *contentLength);

            observer->BodyAvailable(infra::MakeContainedSharedObject(bodyReader->countingReader, bodyReaderAccess.MakeShared(bodyReader)));
        }
    }

    void HttpClientImpl::BodyReaderDestroyed()
    {
        ConnectionObserver::Subject().AckReceived();
        *contentLength -= bodyReader->countingReader.TotalRead();
        bodyReader = infra::none;

        if (*contentLength == 0)
            BodyComplete();
    }

    void HttpClientImpl::BodyComplete()
    {
        contentLength = infra::none;
        response = infra::none;
        observer->BodyComplete();
    }

    void HttpClientImpl::ExecuteRequest(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers)
    {
        request.Emplace(verb, hostname, requestTarget, headers);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers)
    {
        request.Emplace(verb, hostname, requestTarget, content, headers);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers)
    {
        request.Emplace(verb, hostname, requestTarget, contentSize, headers);
        streamingContentSize = contentSize;
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::AbortAndDestroy()
    {
        bodyReaderAccess.SetAction([]() {});
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    HttpClientImpl::HttpResponseParser::HttpResponseParser(HttpClientImpl& httpClient, infra::BoundedString& headerBuffer)
        : httpClient(httpClient)
        , headerBuffer(headerBuffer)
    {}

    void HttpClientImpl::HttpResponseParser::DataReceived(infra::StreamReaderWithRewinding& reader)
    {
        if (!statusParsed)
            ParseStatusLine(reader);

        if (!Error())
            ParseHeaders(reader);
    }

    bool HttpClientImpl::HttpResponseParser::Done() const
    {
        return done;
    }

    bool HttpClientImpl::HttpResponseParser::Error() const
    {
        return error;
    }

    uint32_t HttpClientImpl::HttpResponseParser::ContentLength() const
    {
        return *contentLength;
    }

    void HttpClientImpl::HttpResponseParser::ParseStatusLine(infra::StreamReaderWithRewinding& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
        stream >> headerBuffer;

        auto crlfPos = headerBuffer.find(crlf);
        if (crlfPos != infra::BoundedString::npos)
        {
            auto statusLine = headerBuffer.substr(0, crlfPos);
            reader.Rewind(statusLine.size() + crlf.size());

            infra::Tokenizer tokenizer(statusLine, ' ');

            auto versionValid = HttpVersionValid(tokenizer.Token(0));
            auto statusCode = HttpStatusCodeFromString(tokenizer.Token(1));
            if (versionValid && statusCode)
                httpClient.StatusAvailable(*statusCode, statusLine);
            else
                SetError();

            statusParsed = true;
        }
    }

    bool HttpClientImpl::HttpResponseParser::HttpVersionValid(infra::BoundedConstString httpVersion)
    {
        static const std::array<infra::BoundedConstString, 2> validVersions{ "HTTP/1.0",  "HTTP/1.1" };
        return std::any_of(validVersions.begin(), validVersions.end(), [&](infra::BoundedConstString validVersion) { return httpVersion == validVersion; });
    }

    void HttpClientImpl::HttpResponseParser::ParseHeaders(infra::StreamReaderWithRewinding& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        while (!done && !stream.Empty())
        {
            auto start = reader.ConstructSaveMarker();

            headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
            stream >> headerBuffer;

            auto crlfPos = headerBuffer.find(crlf);
            if (crlfPos != infra::BoundedString::npos)
            {
                auto headerLine = headerBuffer.substr(0, crlfPos);
                reader.Rewind(start + headerLine.size() + crlf.size());

                if (headerLine.empty() && headerBuffer.size() > crlfPos)
                {
                    error = contentLength == infra::none;
                    done = true;
                    return;
                }

                auto header = HeaderFromString(headerLine);
                if (header.Field() == "Content-Length")
                {
                    contentLength = 0;
                    infra::StringInputStream contentLengthStream(header.Value());
                    contentLengthStream >> *contentLength;
                }
                else
                    httpClient.observer->HeaderAvailable(header);
            }
            else if (headerBuffer.full())
                SetError();
            else
            {
                reader.Rewind(start);
                break;
            }
        }
    }

    HttpHeader HttpClientImpl::HttpResponseParser::HeaderFromString(infra::BoundedConstString header)
    {
        infra::Tokenizer tokenizer(header, ':');
        auto value = tokenizer.TokenAndRest(1);
        auto headerBegin = value.find_first_not_of(' ');

        return{ tokenizer.Token(0), headerBegin != infra::BoundedString::npos ? value.substr(headerBegin) : "" };
    }

    void HttpClientImpl::HttpResponseParser::SetError()
    {
        done = true;
        error = true;
    }

    HttpClientImpl::BodyReader::BodyReader(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader, uint32_t contentLength)
        : reader(reader)
        , limitedReader(*reader, contentLength)
    {}
}
