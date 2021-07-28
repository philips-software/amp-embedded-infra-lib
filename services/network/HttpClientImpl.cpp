#include "services/network/HttpClientImpl.hpp"
#include "infra/stream/CountingOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"

namespace services
{
    HttpClientImpl::HttpClientImpl(infra::BoundedConstString hostname)
        : hostname(hostname)
        , bodyReaderAccess(infra::emptyFunction)
        , sendingState(infra::InPlaceType<SendingStateRequest>(), *this)
        , nextState(infra::InPlaceType<SendingStateRequest>(), *this)
    {}

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

    void HttpClientImpl::Close()
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

    void HttpClientImpl::Detaching()
    {
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
        else
            Observer().HeaderAvailable(header);
    }

    void HttpClientImpl::ExpectResponse()
    {
        response.Emplace(static_cast<HttpHeaderParserObserver&>(*this));
    }

    void HttpClientImpl::HandleData()
    {
        if (!response->Done())
        {
            auto reader = ConnectionObserver::Subject().ReceiveStream();

            infra::WeakPtr<services::ConnectionObserver> self = services::ConnectionObserver::Subject().ObserverPtr();

            response->DataReceived(*reader);

            if (!self.lock())   // DataReceived may close the connection
                return;

            ConnectionObserver::Subject().AckReceived();

            if (response->Done())
            {
                if (contentLength == infra::none && (statusCode == HttpStatusCode::Continue
                    || statusCode == HttpStatusCode::SwitchingProtocols
                    || statusCode == HttpStatusCode::NoContent
                    || statusCode == HttpStatusCode::NotModified))
                    contentLength = 0;
            }
        }

        if (response->Done())
        {
            if (!response->Error() && contentLength != infra::none)
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
        contentLength = infra::none;
        firstChunk = true;
        response = infra::none;
        Observer().BodyComplete();
    }

    bool HttpClientImpl::ReadChunkLength()
    {
        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader, infra::softFail);

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
            reader = nullptr;

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
        Observer().FillContent(stream.Writer());
        return stream.Writer().Processed();
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
        client.request->Write(stream);
        client.request = infra::none;
        writer = nullptr;

        NextState();
    }

    void HttpClientImpl::SendingStateRequest::Activate()
    {
        client.ExpectResponse();
    }

    HttpClientImpl::SendingStateForwardSendStream::SendingStateForwardSendStream(HttpClientImpl& client, std::size_t contentSize)
        : SendingState(client)
        , contentSize(contentSize)
    {}

    HttpClientImpl::SendingStateForwardSendStream::SendingStateForwardSendStream(const SendingStateForwardSendStream& other)
        : SendingState(other)
        , contentSize(other.contentSize)
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

        client.Observer().SendStreamAvailable(forwardStreamAccess.MakeShared(*forwardStreamPtr));
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
        client.Observer().FillContent(windowWriter);
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
