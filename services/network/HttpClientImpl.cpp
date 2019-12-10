#include "services/network/HttpClientImpl.hpp"
#include "infra/stream/CountingOutputStream.hpp"
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
        , requestTarget(requestTarget.empty() ? "/" : requestTarget)
        , content(content)
        , hostHeader("host", hostname)
        , headers(headers)
    {
        if (!content.empty())
            AddContentLength(content.size());
    }

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers)
        : verb(verb)
        , requestTarget(requestTarget.empty() ? "/" : requestTarget)
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

    HttpClientImpl::HttpClientImpl(infra::BoundedConstString hostname)
        : hostname(hostname)
        , bodyReaderAccess(infra::emptyFunction)
        , sendingState(infra::InPlaceType<SendingStateRequest>(), *this)
        , nextState(infra::InPlaceType<SendingStateRequest>(), *this)
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

    void HttpClientImpl::Post(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::post, requestTarget, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, content, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, contentSize, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, headers);
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
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    Connection& HttpClientImpl::GetConnection()
    {
        return ConnectionObserver::Subject();
    }

    void HttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendingState->SendStreamAvailable(std::move(writer));
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
        bodyReaderAccess.SetAction(infra::emptyFunction);
        GetObserver().ClosingConnection();
        observer->Detach();
        observer = nullptr;
    }

    void HttpClientImpl::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        observer->StatusAvailable(code);
    }

    void HttpClientImpl::ExpectResponse()
    {
        response.Emplace(*this);
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
        nextState.Emplace<SendingStateForwardSendStream>(*this, contentSize);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers)
    {
        auto contentSize = ReadContentSizeFromObserver();
        request.Emplace(verb, hostname, requestTarget, contentSize, headers);
        nextState.Emplace<SendingStateForwardFillContent>(*this, contentSize);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    uint32_t HttpClientImpl::ReadContentSizeFromObserver() const
    {
        infra::DataOutputStream::WithWriter<infra::CountingStreamWriter> stream;
        GetObserver().FillContent(stream.Writer());
        return stream.Writer().Processed();
    }

    void HttpClientImpl::AbortAndDestroy()
    {
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    HttpClientImpl::HttpResponseParser::HttpResponseParser(HttpClientImpl& httpClient)
        : httpClient(httpClient)
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
        infra::BoundedString::WithStorage<512> headerBuffer;
        headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
        stream >> headerBuffer;

        auto crlfPos = headerBuffer.find(crlf);
        if (crlfPos != infra::BoundedString::npos)
        {
            auto statusLine = headerBuffer.substr(0, crlfPos);
            reader.Rewind(statusLine.size() + crlf.size());

            infra::Tokenizer tokenizer(statusLine, ' ');

            auto versionValid = HttpVersionValid(tokenizer.Token(0));
            auto optionalStatusCode = HttpStatusCodeFromString(tokenizer.Token(1));
            if (versionValid && optionalStatusCode)
            {
                statusCode = *optionalStatusCode;
                httpClient.StatusAvailable(statusCode, statusLine);
            }
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
        infra::BoundedString::WithStorage<512> headerBuffer;
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
                    if (statusCode == HttpStatusCode::Continue
                        || statusCode == HttpStatusCode::SwitchingProtocols
                        || statusCode == HttpStatusCode::NoContent
                        || statusCode == HttpStatusCode::NotModified)
                        contentLength = 0;
                    error = contentLength == infra::none;
                    done = true;
                    return;
                }

                auto header = HeaderFromString(headerLine);
                if (infra::CaseInsensitiveCompare(header.Field(), "Content-Length"))
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
        return{ tokenizer.Token(0), infra::TrimLeft(tokenizer.TokenAndRest(1)) };
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

    HttpClientImpl::SendingState::SendingState(HttpClientImpl& client)
        : client(client)
    {}

    void HttpClientImpl::SendingState::NextState()
    {
        auto& client = this->client;

        client.sendingState = client.nextState;
        client.nextState.Emplace<SendingStateRequest>(client);
        client.sendingState->Activate();
    }

    HttpClientImpl::SendingStateRequest::SendingStateRequest(HttpClientImpl& client)
        : SendingState(client)
    {}

    void HttpClientImpl::SendingStateRequest::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        client.request->Write(stream);
        client.request = infra::none;
        writer = nullptr;

        NextState();
    }

    void HttpClientImpl::SendingStateRequest::Activate()
    {
        client.ExpectResponse();
    }

    HttpClientImpl::SendingStateForwardSendStream::SendingStateForwardSendStream(const SendingStateForwardSendStream& other)
        : SendingState(other)
        , contentSize(other.contentSize)
    {}

    HttpClientImpl::SendingStateForwardSendStream::SendingStateForwardSendStream(HttpClientImpl& client, std::size_t contentSize)
        : SendingState(client)
        , contentSize(contentSize)
    {}

    void HttpClientImpl::SendingStateForwardSendStream::Activate()
    {
        if (contentSize != 0)
            client.ConnectionObserver::Subject().RequestSendStream(std::min(contentSize, client.ConnectionObserver::Subject().MaxSendStreamSize()));
        else
            NextState();
    }

    void HttpClientImpl::SendingStateForwardSendStream::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        forwardStreamPtr = std::move(writer);
        auto available = forwardStreamPtr->Available();
        forwardStreamAccess.SetAction([this, available]()
        {
            contentSize -= available - forwardStreamPtr->Available();

            forwardStreamPtr = nullptr;

            Activate();
        });

        client.observer->SendStreamAvailable(forwardStreamAccess.MakeShared(*forwardStreamPtr));
    }

    HttpClientImpl::SendingStateForwardFillContent::SendingStateForwardFillContent(HttpClientImpl& client, std::size_t contentSize)
        : SendingState(client)
        , contentSize(contentSize)
    {}

    void HttpClientImpl::SendingStateForwardFillContent::Activate()
    {
        if (contentSize != 0)
            client.ConnectionObserver::Subject().RequestSendStream(std::min(contentSize, client.ConnectionObserver::Subject().MaxSendStreamSize()));
        else
            NextState();
    }

    void HttpClientImpl::SendingStateForwardFillContent::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        WindowWriter windowWriter(*writer, processed, writer->Available());
        client.observer->FillContent(windowWriter);
        contentSize -= windowWriter.Processed();
        processed += windowWriter.Processed();
        writer = nullptr;
        Activate();
    }

    HttpClientImpl::SendingStateForwardFillContent::WindowWriter::WindowWriter(infra::StreamWriter& writer, std::size_t start, std::size_t limit)
        : writer(writer)
        , start(start)
        , limit(limit)
    {}

    std::size_t HttpClientImpl::SendingStateForwardFillContent::WindowWriter::Processed() const
    {
        return processed;
    }

    void HttpClientImpl::SendingStateForwardFillContent::WindowWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        auto discard = std::min(range.size(), start);
        range = infra::Head(infra::DiscardHead(range, discard), limit);
        start -= discard;

        if (!range.empty())
        {
            writer.Insert(range, errorPolicy);
            processed += range.size();
        }
    }

    std::size_t HttpClientImpl::SendingStateForwardFillContent::WindowWriter::Available() const
    {
        return std::numeric_limits<std::size_t>::max();
    }
}
