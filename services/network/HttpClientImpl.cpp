#include "services/network/HttpClientImpl.hpp"
#include "infra/stream/CountingOutputStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/Compatibility.hpp"

namespace services
{
    HttpClientImpl::HttpClientImpl(infra::BoundedConstString hostname)
        : hostname(hostname)
        , bodyReaderAccess(infra::emptyFunction)
        , sendingState(infra::InPlaceType<SendingStateRequest>(), *this)
        , nextState(infra::InPlaceType<SendingStateRequest>(), *this)
    {}

    void HttpClientImpl::Retarget(infra::BoundedConstString hostname)
    {
        this->hostname = hostname;
        Reset();
        AbortAndDestroy();
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

    void HttpClientImpl::Post(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::post, requestTarget, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, content, headers);
    }

    void HttpClientImpl::Put(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::put, requestTarget, headers);
    }

    void HttpClientImpl::Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::patch, requestTarget, content, headers);
    }

    void HttpClientImpl::Patch(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::patch, requestTarget, headers);
    }

    void HttpClientImpl::Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        ExecuteRequestWithContent(HttpVerb::delete_, requestTarget, content, headers);
    }

    void HttpClientImpl::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    void HttpClientImpl::CloseConnection()
    {
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    Connection& HttpClientImpl::GetConnection()
    {
        return ConnectionObserver::Subject();
    }

    void HttpClientImpl::Attached()
    {
        infra::WeakPtr<HttpClientImpl> self = infra::StaticPointerCast<HttpClientImpl>(services::ConnectionObserver::Subject().ObserverPtr());
        bodyReaderAccess.SetAction([self]()
            {
                if (auto sharedSelf = self.lock())
                    sharedSelf->BodyReaderDestroyed();
            });
    }

    void HttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendingState->SendStreamAvailable(std::move(writer));
    }

    void HttpClientImpl::DataReceived()
    {
        if (bodyReader != infra::none)
            Observer().BodyAvailable(infra::MakeContainedSharedObject(bodyReader->countingReader, bodyReaderAccess.MakeShared(bodyReader)));
        else
        {
            if (response)
                HandleData();
            else
                AbortAndDestroy();
        }
    }

    void HttpClientImpl::Close()
    {
        if (HttpClient::IsAttached())
            Observer().CloseRequested();
        else
            ConnectionObserver::Subject().CloseAndDestroy();
    }

    void HttpClientImpl::Detaching()
    {
        reader = nullptr;
        bodyReaderAccess.SetAction(infra::emptyFunction);
        if (HttpClient::IsAttached())
            HttpClient::Detach();
    }

    void HttpClientImpl::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        statusCode = code;
        Observer().StatusAvailable(code);
    }

    void HttpClientImpl::HeaderAvailable(HttpHeader header)
    {
        if (infra::CaseInsensitiveCompare(header.Field(), "Content-Length"))
        {
            contentLength = 0;
            infra::StringInputStream contentLengthStream(header.Value());
            contentLengthStream >> *contentLength;
        }
        else if (infra::CaseInsensitiveCompare(header.Field(), "Transfer-Encoding") && header.Value() == "chunked")
        {
            contentLength = 0;
            chunkedEncoding = true;
        }
        else if (HttpClient::IsAttached())
            Observer().HeaderAvailable(header);
    }

    void HttpClientImpl::HeaderParsingDone(bool error)
    {
        headerParsingDone = true;
        headerParsingError = error;
    }

    void HttpClientImpl::ExpectResponse()
    {
        response.Emplace(static_cast<HttpHeaderParserObserver&>(*this));
    }

    void HttpClientImpl::HandleData()
    {
        if (!headerParsingDone)
        {
            reader = ConnectionObserver::Subject().ReceiveStream();

            infra::WeakPtr<services::ConnectionObserver> self = services::ConnectionObserver::Subject().ObserverPtr();

            response->DataReceived(*reader);

            if (!self.lock() || !ConnectionObserver::IsAttached()) // DataReceived may close the connection
            {
                if (!ConnectionObserver::IsAttached())
                    reader = nullptr;
                return;
            }

            ConnectionObserver::Subject().AckReceived();
            reader = nullptr;

            if (headerParsingDone)
            {
                if (contentLength == infra::none && (statusCode == HttpStatusCode::Continue || statusCode == HttpStatusCode::SwitchingProtocols || statusCode == HttpStatusCode::NoContent || statusCode == HttpStatusCode::NotModified))
                    contentLength = 0;
            }
        }

        if (headerParsingDone)
        {
            if (!headerParsingError && contentLength != infra::none)
                BodyReceived();
            else
                AbortAndDestroy();
        }
    }

    void HttpClientImpl::BodyReceived()
    {
        bool repeat = true;
        while (repeat)
        {
            if (contentLength == 0)
            {
                if (chunkedEncoding)
                    repeat = ReadChunkLength();
                else
                {
                    repeat = false;
                    BodyComplete();
                }
            }
            else
            {
                bodyReader.Emplace(ConnectionObserver::Subject().ReceiveStream(), *contentLength);

                if (HttpClient::IsAttached())
                    Observer().BodyAvailable(infra::MakeContainedSharedObject(bodyReader->countingReader, bodyReaderAccess.MakeShared(bodyReader)));
                repeat = chunkedEncoding && contentLength == 0;
            }
        }
    }

    void HttpClientImpl::BodyReaderDestroyed()
    {
        ConnectionObserver::Subject().AckReceived();
        *contentLength -= bodyReader->countingReader.TotalRead();
        bodyReader = infra::none;

        if (*contentLength == 0 && !chunkedEncoding)
            BodyComplete();
    }

    void HttpClientImpl::BodyComplete()
    {
        Reset();
        Observer().BodyComplete();
    }

    void HttpClientImpl::Reset()
    {
        contentLength = infra::none;
        firstChunk = true;
        headerParsingDone = false;
        headerParsingError = false;
        response = infra::none;
    }

    bool HttpClientImpl::ReadChunkLength()
    {
        auto chunkSizeReader = ConnectionObserver::Subject().ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*chunkSizeReader, infra::softFail);

        uint32_t chunkLength = 0;
        char r;
        char n;

        if (!firstChunk)
        {
            stream >> r >> n;
            if (stream.Failed())
                return false;

            if (r != '\r' || n != '\n')
            {
                AbortAndDestroy();
                return false;
            }
        }

        stream >> infra::hex >> chunkLength >> r;

        if (!stream.Failed())
        {
            if (r == ';')
            {
                while (!stream.Failed() && r != '\r')
                    stream >> r;
            }

            stream >> n;

            if (stream.Failed())
                return false;

            if (r != '\r' || n != '\n')
            {
                AbortAndDestroy();
                return false;
            }

            ConnectionObserver::Subject().AckReceived();
            *contentLength = chunkLength;
            firstChunk = false;
            chunkSizeReader = nullptr;

            if (chunkLength == 0)
                chunkedEncoding = false;

            return true;
        }
        else
            return false;
    }

    void HttpClientImpl::ExecuteRequest(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers)
    {
        request.Emplace(verb, hostname, requestTarget, headers);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers)
    {
        request.Emplace(verb, hostname, requestTarget, content, headers);
        ConnectionObserver::Subject().RequestSendStream(std::min(request->Size(), ConnectionObserver::Subject().MaxSendStreamSize()));
    }

    void HttpClientImpl::ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers)
    {
        request.Emplace(verb, hostname, requestTarget, headers, chunked);
        nextState.Emplace<SendingStateForwardSendStream>(*this);
        ConnectionObserver::Subject().RequestSendStream(request->Size());
    }

    void HttpClientImpl::AbortAndDestroy()
    {
        ConnectionObserver::Subject().AbortAndDestroy();
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
        client.request->Consume(client.request->Write(stream));

        if (client.request->Size() == 0)
        {
            client.request = infra::none;
            writer = nullptr;

            NextState();
        }
        else
        {
            writer = nullptr;
            client.ConnectionObserver::Subject().RequestSendStream(std::min(client.request->Size(), client.ConnectionObserver::Subject().MaxSendStreamSize()));
        }
    }

    void HttpClientImpl::SendingStateRequest::Activate()
    {
        client.ExpectResponse();
    }

    HttpClientImpl::SendingStateForwardSendStream::SendingStateForwardSendStream(HttpClientImpl& client)
        : SendingState(client)
    {}

    HttpClientImpl::SendingStateForwardSendStream::SendingStateForwardSendStream(const SendingStateForwardSendStream& other)
        : SendingState(other.client)
        , first(other.first)
        , done(other.done)
    {
        assert(chunkWriter.Allocatable());
        assert(other.chunkWriter.Allocatable());
    }

    void HttpClientImpl::SendingStateForwardSendStream::Activate()
    {
        if (!done)
            client.ConnectionObserver::Subject().RequestSendStream(client.ConnectionObserver::Subject().MaxSendStreamSize());
        else
            NextState();
    }

    void HttpClientImpl::SendingStateForwardSendStream::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        chunkWriter.OnAllocatable([this]()
            {
                Activate();
            });

        client.Observer().SendStreamAvailable(chunkWriter.Emplace(*this, std::move(writer)));
    }

    HttpClientImpl::SendingStateForwardSendStream::ChunkWriter::ChunkWriter(SendingStateForwardSendStream& state, infra::SharedPtr<infra::StreamWriter>&& writer)
        : infra::LimitedStreamWriter(*writer, std::min<std::size_t>(writer->Available(), 65536) - 8)
        , state(state)
        , writer(std::move(writer))
        , start(ConstructSaveMarker())
    {}

    HttpClientImpl::SendingStateForwardSendStream::ChunkWriter::~ChunkWriter()
    {
        auto written = ConstructSaveMarker() - start;

        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        infra::SavedMarkerTextStream insertStream(stream, start);

        if (!state.first)
            insertStream << "\r\n";
        insertStream << infra::hex << written << "\r\n";

        state.first = false;

        if (written == 0)
        {
            state.done = true;

            insertStream << "\r\n";
        }
    }

    HttpClientImplWithRedirection::HttpClientImplWithRedirection(infra::BoundedString redirectedUrlStorage, infra::BoundedConstString hostname, ConnectionFactoryWithNameResolver& connectionFactory)
        : HttpClientImpl(hostname)
        , redirectedUrlStorage(redirectedUrlStorage)
        , connectionFactory(connectionFactory)
    {}

    HttpClientImplWithRedirection::~HttpClientImplWithRedirection()
    {
        if (connecting)
            connectionFactory.CancelConnect(*this);
    }

    void HttpClientImplWithRedirection::Attached()
    {
        if (!redirecting)
            HttpClientImpl::Attached();
        else
            (*query)->Execute(*this, redirectedPath);
    }

    void HttpClientImplWithRedirection::Detaching()
    {
        if (!redirecting)
            HttpClientImpl::Detaching();
    }

    void HttpClientImplWithRedirection::Get(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryGet>(), headers);

        HttpClientImpl::Get(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Head(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryHead>(), headers);

        HttpClientImpl::Head(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Connect(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryConnect>(), headers);

        HttpClientImpl::Connect(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Options(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryOptions>(), headers);

        HttpClientImpl::Options(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryPost>(), content, headers);

        HttpClientImpl::Post(requestTarget, content, headers);
    }

    void HttpClientImplWithRedirection::Post(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryPostChunked>(), headers);

        HttpClientImpl::Post(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryPut>(), content, headers);

        HttpClientImpl::Put(requestTarget, content, headers);
    }

    void HttpClientImplWithRedirection::Put(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryPutChunked>(), headers);

        HttpClientImpl::Put(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryPatch>(), content, headers);

        HttpClientImpl::Patch(requestTarget, content, headers);
    }

    void HttpClientImplWithRedirection::Patch(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryPatchChunked>(), headers);

        HttpClientImpl::Patch(requestTarget, headers);
    }

    void HttpClientImplWithRedirection::Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        if (query == infra::none)
            query.Emplace(infra::InPlaceType<QueryDelete>(), content, headers);

        HttpClientImpl::Delete(requestTarget, content, headers);
    }

    infra::BoundedConstString HttpClientImplWithRedirection::Hostname() const
    {
        return redirectedHostname;
    }

    uint16_t HttpClientImplWithRedirection::Port() const
    {
        return redirectedPort;
    }

    void HttpClientImplWithRedirection::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> client)>&& createdObserver)
    {
        connecting = false;
        createdObserver(std::move(self));
        redirecting = false;
    }

    void HttpClientImplWithRedirection::ConnectionFailed(ConnectFailReason reason)
    {
        connecting = false;
        redirecting = false;
        RedirectFailed();
        self = nullptr;
    }

    void HttpClientImplWithRedirection::StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        switch (code)
        {
            case HttpStatusCode::MovedPermanently:  // 301
            case HttpStatusCode::SeeOther:          // 303
            case HttpStatusCode::TemporaryRedirect: // 307
            case HttpStatusCode::PermanentRedirect: // 308
                if (redirectionCount != maxRedirection)
                {
                    ++redirectionCount;
                    redirecting = true;
                    break;
                }
                EMIL_FALLTHROUGH;
            default:
                HttpClientImpl::StatusAvailable(code, statusLine);
                break;
        }
    }

    void HttpClientImplWithRedirection::HeaderAvailable(HttpHeader header)
    {
        if (redirecting)
        {
            if (infra::CaseInsensitiveCompare(header.Field(), "Location"))
                redirectedUrlStorage.assign(header.Value().substr(0, redirectedUrlStorage.max_size()));
        }
        else
            HttpClientImpl::HeaderAvailable(header);
    }

    void HttpClientImplWithRedirection::HeaderParsingDone(bool error)
    {
        if (redirecting)
        {
            if (!error)
                Redirect();
            else
                RedirectFailed();
        }
        else
            HttpClientImpl::HeaderParsingDone(error);
    }

    void HttpClientImplWithRedirection::Redirect()
    {
        Redirecting(redirectedUrlStorage);

        redirectedHostname = HostFromUrl(redirectedUrlStorage);
        redirectedPort = PortFromUrl(redirectedUrlStorage).ValueOr(PortFromScheme(SchemeFromUrl(redirectedUrlStorage)).ValueOr(80));
        redirectedPath = PathFromUrl(redirectedUrlStorage);

        self = infra::StaticPointerCast<HttpClientImplWithRedirection>(Subject().ObserverPtr());

        Retarget(Hostname());

        connecting = true;
        connectionFactory.Connect(*this);
    }

    void HttpClientImplWithRedirection::RedirectFailed()
    {
        infra::WeakPtr<services::ConnectionObserver> localSelf = services::ConnectionObserver::IsAttached() ? services::ConnectionObserver::Subject().ObserverPtr() : nullptr;

        HttpClientImpl::StatusAvailable(HttpStatusCode::NotFound, "Redirection failed");

        if (localSelf.lock() || (self != nullptr && HttpClient::IsAttached()))
            HttpClientImpl::HeaderParsingDone(true);

        if (localSelf.lock() || (self != nullptr && HttpClient::IsAttached()))
            HttpClientImpl::BodyComplete();
    }

    infra::Optional<uint16_t> HttpClientImplWithRedirection::PortFromScheme(infra::BoundedConstString scheme) const
    {
        if (infra::CaseInsensitiveCompare(scheme, "http"))
            return infra::MakeOptional<uint16_t>(80);
        if (infra::CaseInsensitiveCompare(scheme, "https"))
            return infra::MakeOptional<uint16_t>(443);
        return infra::none;
    }

    HttpClientImplWithRedirection::QueryGet::QueryGet(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryGet::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Get(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryHead::QueryHead(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryHead::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Head(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryConnect::QueryConnect(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryConnect::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Connect(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryOptions::QueryOptions(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryOptions::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Options(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryPost::QueryPost(infra::BoundedConstString content, HttpHeaders headers)
        : content(content)
        , headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryPost::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Post(requestTarget, content, headers);
    }

    HttpClientImplWithRedirection::QueryPostChunked::QueryPostChunked(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryPostChunked::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Post(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryPut::QueryPut(infra::BoundedConstString content, HttpHeaders headers)
        : content(content)
        , headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryPut::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Put(requestTarget, content, headers);
    }

    HttpClientImplWithRedirection::QueryPutChunked::QueryPutChunked(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryPutChunked::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Put(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryPatch::QueryPatch(infra::BoundedConstString content, HttpHeaders headers)
        : headers(headers)
        , content(content)
    {}

    void HttpClientImplWithRedirection::QueryPatch::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Patch(requestTarget, content, headers);
    }

    HttpClientImplWithRedirection::QueryPatchChunked::QueryPatchChunked(HttpHeaders headers)
        : headers(headers)
    {}

    void HttpClientImplWithRedirection::QueryPatchChunked::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Patch(requestTarget, headers);
    }

    HttpClientImplWithRedirection::QueryDelete::QueryDelete(infra::BoundedConstString content, HttpHeaders headers)
        : headers(headers)
        , content(content)
    {}

    void HttpClientImplWithRedirection::QueryDelete::Execute(HttpClient& client, infra::BoundedConstString requestTarget)
    {
        client.Delete(requestTarget, content, headers);
    }
}
