#include "services/network/HttpClientImpl.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    namespace
    {
        // Naming is according to rfc7230
        const infra::BoundedConstString httpVersion = "HTTP/1.1";
        const infra::BoundedConstString sp = " ";
        const infra::BoundedConstString crlf = "\r\n";

        class CountingStreamReader
            : public infra::StreamReader
            , private infra::StreamErrorPolicy
        {
        public:
            CountingStreamReader(infra::StreamReader& reader);

            uint32_t TotalRead() const;

            // Implementation of StreamReader
            virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            virtual bool Empty() const override;
            virtual std::size_t Available() const override;

            // Implementation of StreamErrorPolicy
            virtual bool Failed() const override;
            virtual void ReportResult(bool ok) override;

        private:
            infra::StreamReader& reader;
            infra::StreamErrorPolicy* otherErrorPolicy = nullptr;
            bool ok = true;
            uint32_t totalRead = 0;
        };

        CountingStreamReader::CountingStreamReader(infra::StreamReader& reader)
            : reader(reader)
        {}

        uint32_t CountingStreamReader::TotalRead() const
        {
            return totalRead;
        }

        void CountingStreamReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
        {
            otherErrorPolicy = &errorPolicy;
            ok = true;
            reader.Extract(range, *this);

            if (ok)
                totalRead += range.size();
            otherErrorPolicy = nullptr;
        }

        uint8_t CountingStreamReader::Peek(infra::StreamErrorPolicy& errorPolicy)
        {
            return reader.Peek(errorPolicy);
        }

        infra::ConstByteRange CountingStreamReader::ExtractContiguousRange(std::size_t max)
        {
            auto result = reader.ExtractContiguousRange(max);
            totalRead += result.size();
            return result;
        }

        infra::ConstByteRange CountingStreamReader::PeekContiguousRange(std::size_t start)
        {
            return reader.PeekContiguousRange(start);
        }

        bool CountingStreamReader::Empty() const
        {
            return reader.Empty();
        }

        std::size_t CountingStreamReader::Available() const
        {
            return reader.Available();
        }

        bool CountingStreamReader::Failed() const
        {
            return otherErrorPolicy->Failed();
        }

        void CountingStreamReader::ReportResult(bool ok)
        {
            otherErrorPolicy->ReportResult(ok);
            this->ok = this->ok && ok;
        }
    }

    HttpRequestFormatter::HttpRequestFormatter(infra::BoundedConstString hostname, infra::BoundedConstString method, infra::BoundedConstString requestTarget, const HttpHeaders headers)
        : HttpRequestFormatter(hostname, method, requestTarget, {}, headers)
    {}

    HttpRequestFormatter::HttpRequestFormatter(infra::BoundedConstString hostname, infra::BoundedConstString method, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers)
        : method(method)
        , requestTarget(requestTarget)
        , content(content)
        , hostHeader("host", hostname)
        , headers(headers)
    {
        if (!content.empty())
        {
            contentLength << content.size();
            contentLengthHeader.Emplace("content-length", contentLength.Storage());
        }
    }

    std::size_t HttpRequestFormatter::Size() const
    {
        return method.size() + requestTarget.size() + httpVersion.size() + HeadersSize() + (2 * crlf.size()) + (2 * sp.size()) + content.size();
    }

    void HttpRequestFormatter::Write(infra::TextOutputStream stream) const
    {
        stream << method << sp << requestTarget << sp << httpVersion << crlf;

        for (auto&& header : headers)
            stream << header << crlf;

        stream << hostHeader << crlf;

        if (contentLengthHeader)
            stream << *contentLengthHeader << crlf;

        stream << crlf;
        stream << content;
    }

    std::size_t HttpRequestFormatter::HeadersSize() const
    {
        std::size_t headerSize = 0;
        for (auto&& header : headers)
            headerSize += (header.Size() + crlf.size());

        if (contentLengthHeader)
            headerSize += contentLengthHeader->Size();

        headerSize += hostHeader.Size();

        return headerSize;
    }

    HttpResponseParser::HttpResponseParser(infra::SharedPtr<HttpClientObserver> observer, infra::BoundedString& headerBuffer)
        : observer(observer)
        , headerBuffer(headerBuffer)
    {}

    void HttpResponseParser::DataReceived(infra::StreamReaderWithRewinding& reader)
    {
        if (!statusParsed)
            ParseStatusLine(reader);

        if (!Error())
            ParseHeaders(reader);
    }

    bool HttpResponseParser::Done() const
    {
        return done;
    }

    bool HttpResponseParser::Error() const
    {
        return error;
    }

    uint32_t HttpResponseParser::ContentLength() const
    {
        return *contentLength;
    }

    void HttpResponseParser::ParseStatusLine(infra::StreamReaderWithRewinding& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
        stream >> headerBuffer;

        auto crlfPos = headerBuffer.find_first_of(crlf);
        if (crlfPos != infra::BoundedString::npos)
        {
            auto statusLine = headerBuffer.substr(0, crlfPos + crlf.size());
            reader.Rewind(statusLine.size());

            infra::Tokenizer tokenizer(statusLine, ' ');

            auto versionValid = HttpVersionValid(tokenizer.Token(0));
            auto statusCode = StatusCodeFromString(tokenizer.Token(1));
            if (versionValid && statusCode)
                observer->StatusAvailable(*statusCode);
            else
                SetError();

            statusParsed = true;
        }
    }

    bool HttpResponseParser::HttpVersionValid(infra::BoundedConstString httpVersion)
    {
        static const std::array<infra::BoundedConstString, 2> validVersions{ "HTTP/1.0",  "HTTP/1.1" };
        return std::any_of(validVersions.begin(), validVersions.end(), [&](infra::BoundedConstString validVersion) { return httpVersion == validVersion; });
    }

    infra::Optional<HttpStatusCode> HttpResponseParser::StatusCodeFromString(infra::BoundedConstString statusCode)
    {
        std::underlying_type<services::HttpStatusCode>::type value = 0;

        for (std::size_t index = 0; index < statusCode.size(); ++index)
            value = value * 10 + statusCode[index] - '0';

        switch (value)
        {
            case 100: return infra::MakeOptional(HttpStatusCode::Continue);
            case 101: return infra::MakeOptional(HttpStatusCode::SwitchingProtocols);
            case 200: return infra::MakeOptional(HttpStatusCode::OK);
            case 201: return infra::MakeOptional(HttpStatusCode::Created);
            case 202: return infra::MakeOptional(HttpStatusCode::Accepted);
            case 203: return infra::MakeOptional(HttpStatusCode::NonAuthorativeInformation);
            case 204: return infra::MakeOptional(HttpStatusCode::NoContent);
            case 205: return infra::MakeOptional(HttpStatusCode::ResetContent);
            case 206: return infra::MakeOptional(HttpStatusCode::PartialContent);
            case 300: return infra::MakeOptional(HttpStatusCode::MultipleChoices);
            case 301: return infra::MakeOptional(HttpStatusCode::MovedPermanently);
            case 302: return infra::MakeOptional(HttpStatusCode::Found);
            case 303: return infra::MakeOptional(HttpStatusCode::SeeOther);
            case 304: return infra::MakeOptional(HttpStatusCode::NotModified);
            case 305: return infra::MakeOptional(HttpStatusCode::UseProxy);
            case 307: return infra::MakeOptional(HttpStatusCode::TemporaryRedirect);
            case 400: return infra::MakeOptional(HttpStatusCode::BadRequest);
            case 401: return infra::MakeOptional(HttpStatusCode::Unauthorized);
            case 402: return infra::MakeOptional(HttpStatusCode::PaymentRequired);
            case 403: return infra::MakeOptional(HttpStatusCode::Forbidden);
            case 404: return infra::MakeOptional(HttpStatusCode::NotFound);
            case 405: return infra::MakeOptional(HttpStatusCode::MethodNotAllowed);
            case 406: return infra::MakeOptional(HttpStatusCode::NotAcceptable);
            case 407: return infra::MakeOptional(HttpStatusCode::ProxyAuthenticationRequired);
            case 408: return infra::MakeOptional(HttpStatusCode::RequestTimeOut);
            case 409: return infra::MakeOptional(HttpStatusCode::Conflict);
            case 410: return infra::MakeOptional(HttpStatusCode::Gone);
            case 411: return infra::MakeOptional(HttpStatusCode::LengthRequired);
            case 412: return infra::MakeOptional(HttpStatusCode::PreconditionFailed);
            case 413: return infra::MakeOptional(HttpStatusCode::RequestEntityTooLarge);
            case 414: return infra::MakeOptional(HttpStatusCode::RequestUriTooLarge);
            case 415: return infra::MakeOptional(HttpStatusCode::UnsupportedMediaType);
            case 416: return infra::MakeOptional(HttpStatusCode::RequestRangeNotSatisfiable);
            case 417: return infra::MakeOptional(HttpStatusCode::ExpectationFailed);
            case 500: return infra::MakeOptional(HttpStatusCode::InternalServerError);
            case 501: return infra::MakeOptional(HttpStatusCode::NotImplemented);
            case 502: return infra::MakeOptional(HttpStatusCode::BadGateway);
            case 503: return infra::MakeOptional(HttpStatusCode::ServiceUnavailable);
            case 504: return infra::MakeOptional(HttpStatusCode::GatewayTimeOut);
            case 505: return infra::MakeOptional(HttpStatusCode::HttpVersionNotSupported);
        }

        return infra::none;
    }

    void HttpResponseParser::ParseHeaders(infra::StreamReaderWithRewinding& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        while (!stream.Empty())
        {
            auto start = reader.ConstructSaveMarker();

            headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
            stream >> headerBuffer;

            auto crlfPos = headerBuffer.find_first_of(crlf);
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
                    observer->HeaderAvailable(header);
            }
            else if (headerBuffer.full())
                error = true;
        }
    }

    HttpHeader HttpResponseParser::HeaderFromString(infra::BoundedConstString header)
    {
        infra::Tokenizer tokenizer(header, ':');
        auto value = tokenizer.TokenAndRest(1);
        auto headerBegin = value.find_first_not_of(' ');

        return{ tokenizer.Token(0), headerBegin != infra::BoundedString::npos ? value.substr(headerBegin) : "" };
    }

    void HttpResponseParser::SetError()
    {
        done = true;
        error = true;
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
        ExecuteRequest("GET", requestTarget, headers);
    }

    void HttpClientImpl::Head(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest("HEAD", requestTarget, headers);
    }

    void HttpClientImpl::Connect(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest("CONNECT", requestTarget, headers);
    }

    void HttpClientImpl::Options(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequest("OPTIONS", requestTarget, headers);
    }

    void HttpClientImpl::Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent("POST", requestTarget, content, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent("PUT", requestTarget, content, headers);
    }

    void HttpClientImpl::Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent("PATCH", requestTarget, content, headers);
    }

    void HttpClientImpl::Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent("DELETE", requestTarget, content, headers);
    }

    void HttpClientImpl::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    void HttpClientImpl::Close()
    {
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void HttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        request->Write(stream);
        request = infra::none;
        response.Emplace(observer, headerBuffer);
        writer = nullptr;
    }

    void HttpClientImpl::DataReceived()
    {
        auto reader = ConnectionObserver::Subject().ReceiveStream();

        if (response)
        {
            if (!response->Done())
            {
                infra::WeakPtr<services::ConnectionObserver> self = services::ConnectionObserver::Subject().Observer();

                response->DataReceived(*reader);

                if (!self.lock())   // DataReceived may close the connection
                    return;

                ConnectionObserver::Subject().AckReceived();
            }

            if (response->Done())
            {
                if (!response->Error())
                {
                    if (!contentLength)
                        contentLength = response->ContentLength();

                    if (!reader->Empty())
                    {
                        infra::LimitedStreamReader limitedReader(*reader, *contentLength);
                        CountingStreamReader countingReader(limitedReader);

                        infra::WeakPtr<services::ConnectionObserver> self = services::ConnectionObserver::Subject().Observer();

                        observer->BodyAvailable(countingReader);

                        if (!self.lock())   // BodyAvailable may close the connection
                            return;

                        ConnectionObserver::Subject().AckReceived();

                        *contentLength -= countingReader.TotalRead();
                    }

                    if (*contentLength == 0)
                    {
                        contentLength = infra::none;
                        observer->BodyComplete();
                    }
                }
                else
                    ConnectionObserver::Subject().AbortAndDestroy();
            }
        }
        else
            ConnectionObserver::Subject().AbortAndDestroy();
    }

    void HttpClientImpl::Connected()
    {
        GetObserver().Connected();
    }

    void HttpClientImpl::ClosingConnection()
    {
        GetObserver().ClosingConnection();
        observer->Detach();
    }

    void HttpClientImpl::ExecuteRequest(infra::BoundedConstString method, infra::BoundedConstString requestTarget, const HttpHeaders headers)
    {
        request.Emplace(hostname, method, requestTarget, headers);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::ExecuteRequestWithContent(infra::BoundedConstString method, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers)
    {
        request.Emplace(hostname, method, requestTarget, content, headers);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    HttpClientConnectorImpl::HttpClientConnectorImpl(infra::BoundedString& headerBuffer, services::ConnectionFactoryWithNameResolver& connectionFactory)
        : headerBuffer(headerBuffer)
        , connectionFactory(connectionFactory)
        , client([this]() { TryConnectWaiting(); })
    {}

    infra::BoundedConstString HttpClientConnectorImpl::Hostname() const
    {
        return clientObserverFactory->Hostname();
    }

    uint16_t HttpClientConnectorImpl::Port() const
    {
        return clientObserverFactory->Port();
    }

    void HttpClientConnectorImpl::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory);
        auto clientPtr = client.Emplace(headerBuffer, Hostname());

        clientObserverFactory->ConnectionEstablished([&clientPtr, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
        {
            if (observer)
            {
                clientPtr->AttachObserver(observer);
                createdObserver(clientPtr);
            }
        });

        clientObserverFactory = nullptr;
    }

    void HttpClientConnectorImpl::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientObserverFactory);

        switch (reason)
        {
            case ConnectFailReason::refused:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::refused);
                break;
            case ConnectFailReason::connectionAllocationFailed:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                break;
            case ConnectFailReason::nameLookupFailed:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
                break;
            default:
                std::abort();
        }

        clientObserverFactory = nullptr;
        TryConnectWaiting();
    }

    void HttpClientConnectorImpl::Connect(HttpClientObserverFactory& factory)
    {
        waitingClientObserverFactories.push_back(factory);
        TryConnectWaiting();
    }

    void HttpClientConnectorImpl::CancelConnect(HttpClientObserverFactory& factory)
    {
        if (clientObserverFactory == &factory)
        {
            connectionFactory.CancelConnect(*this);
            clientObserverFactory = nullptr;
        }
        else
            waitingClientObserverFactories.erase(factory);

        TryConnectWaiting();
    }

    void HttpClientConnectorImpl::TryConnectWaiting()
    {
        if (clientObserverFactory == nullptr && client.Allocatable() && !waitingClientObserverFactories.empty())
        {
            clientObserverFactory = &waitingClientObserverFactories.front();
            waitingClientObserverFactories.pop_front();
            connectionFactory.Connect(*this);
        }
    }
}
