#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/Endian.hpp"
#include "mbedtls/sha1.h"
#include "services/network/HttpServer.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include <cassert>

namespace services
{
    WebSocketClientConnectionObserver::WebSocketClientConnectionObserver(infra::BoundedConstString path, services::Connection& connection)
        : streamWriter([this]() { StreamWriterAllocatable(); })
    {
        connection.GetObserver().Detach();
        services::ConnectionObserver::Attach(connection);
    }

    WebSocketClientConnectionObserver::~WebSocketClientConnectionObserver()
    {
        ResetOwnership();
    }

    void WebSocketClientConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (pongStreamRequested)
        {
            SendPong(std::move(writer));
            TryAllocateSendStream();
        }
        else
        {
            assert(observerStreamRequested);
            assert(requestedSendSize != infra::none);
            assert(!streamWriter);
            Connection::GetObserver().SendStreamAvailable(streamWriter.Emplace(std::move(writer), *infra::PostAssign(requestedSendSize, infra::none)));
        }
    }

    void WebSocketClientConnectionObserver::DataReceived()
    {
        auto before = unackedReadAvailable;
        DiscoverData();
        TryAllocateSendStream();

        if (before != unackedReadAvailable)
            GetObserver().DataReceived();
    }

    void WebSocketClientConnectionObserver::ClosingConnection()
    {
        ResetOwnership();
    }

    void WebSocketClientConnectionObserver::RequestSendStream(std::size_t sendSize)
    {
        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t WebSocketClientConnectionObserver::MaxSendStreamSize() const
    {
        return ConnectionObserver::Subject().MaxSendStreamSize() - 8;
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> WebSocketClientConnectionObserver::ReceiveStream()
    {
        return streamReader.Emplace(*this);
    }

    void WebSocketClientConnectionObserver::AckReceived()
    {
        SearchForPingRequests();
        TryAllocateSendStream();

        streamReader->AckReceived();
        ConnectionObserver::Subject().AckReceived();

        saveAtEndOfDiscovery = 0;
        unackedReadAvailable = availableInCurrentFrame;
        skipDiscoveryPayload = availableInCurrentFrame;
        DiscoverData();
    }

    void WebSocketClientConnectionObserver::CloseAndDestroy()
    {
        Close();
    }

    void WebSocketClientConnectionObserver::AbortAndDestroy()
    {
        Abort();
    }

    void WebSocketClientConnectionObserver::StreamWriterAllocatable()
    {
        observerStreamRequested = false;
        TryAllocateSendStream();
    }

    void WebSocketClientConnectionObserver::TryAllocateSendStream()
    {
        if (!pongStreamRequested && !observerStreamRequested)
        {
            if (pongRequested)
            {
                pongRequested = false;
                pongStreamRequested = true;
                ConnectionObserver::Subject().RequestSendStream(8 + pongBuffer.max_size());
            }
            else if (requestedSendSize != infra::none)
            {
                observerStreamRequested = true;
                ConnectionObserver::Subject().RequestSendStream(*requestedSendSize + 8);
            }
        }
    }

    void WebSocketClientConnectionObserver::DiscoverData()
    {
        if (!streamReader)
            DiscoverData(ConnectionObserver::Subject().ReceiveStream());
        else
            DiscoverData(streamReader->Reader());
    }

    void WebSocketClientConnectionObserver::DiscoverData(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
    {
        auto save = reader->ConstructSaveMarker();
        reader->Rewind(saveAtEndOfDiscovery);

        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        SkipPayloadInDiscovery(stream);
        saveAtEndOfDiscovery = reader->ConstructSaveMarker();
        while (skipDiscoveryPayload == 0 && !stream.Failed())
        {
            uint8_t opcode;
            uint32_t payloadLength;
            std::tie(opcode, payloadLength) = ReadOpcodeAndPayloadLength(stream);

            if (!stream.Failed())
            {
                if (static_cast<WebSocketOpCode>(opcode) == WebSocketOpCode::opCodeBin)
                    unackedReadAvailable += payloadLength;

                if (static_cast<WebSocketOpCode>(opcode) == WebSocketOpCode::opCodePing && unackedReadAvailable == 0)
                {
                    if (stream.Available() < payloadLength)
                        break;

                    pongBuffer.resize(std::min<std::size_t>(payloadLength, pongBuffer.max_size()));
                    stream >> infra::MakeRange(pongBuffer);
                    pongRequested = true;
                    payloadLength -= pongBuffer.size();
                }

                skipDiscoveryPayload = payloadLength;
                SkipPayloadInDiscovery(stream);
                saveAtEndOfDiscovery = reader->ConstructSaveMarker();
            }
        }

        reader->Rewind(save);
    }

    void WebSocketClientConnectionObserver::SkipPayloadInDiscovery(infra::DataInputStream& stream)
    {
        while (skipDiscoveryPayload != 0)
        {
            auto range = stream.Reader().ExtractContiguousRange(skipDiscoveryPayload);
            skipDiscoveryPayload -= range.size();

            if (range.empty())
                break;
        }
    }

    void WebSocketClientConnectionObserver::SearchForPingRequests()
    {
        auto reader = streamReader->Reader();
        auto save = reader->ConstructSaveMarker();
        reader->Rewind(0);
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);

        while (availableInCurrentFrame <= reader->Available() && reader->ConstructSaveMarker() < save)
        {
            reader->Rewind(reader->ConstructSaveMarker() + availableInCurrentFrame);
            uint8_t opcode;
            uint32_t payloadLength;
            std::tie(opcode, payloadLength) = ReadOpcodeAndPayloadLength(stream);

            if (!stream.Failed() && stream.Available() >= payloadLength)
            {
                if (static_cast<WebSocketOpCode>(opcode) == WebSocketOpCode::opCodePing)
                {
                    pongBuffer.resize(std::min<std::size_t>(payloadLength, pongBuffer.max_size()));
                    stream >> infra::MakeRange(pongBuffer);
                    pongRequested = true;
                    payloadLength -= pongBuffer.size();
                }

                Skip(*reader, payloadLength);
            }
        }

        reader->Rewind(save);
    }

    void WebSocketClientConnectionObserver::SendPong(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        pongStreamRequested = false;

        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << static_cast<uint8_t>(static_cast<uint8_t>(WebSocketOpCode::opCodePong) | static_cast<uint8_t>(WebSocketMask::finMask));
        stream << static_cast<uint8_t>(static_cast<uint8_t>(pongBuffer.size()) | static_cast<uint8_t>(WebSocketMask::payloadMask));
        stream << WebSocketMaskingKey();
        stream << infra::MakeRange(pongBuffer);

        writer = nullptr;
    }

    std::pair<uint8_t, uint32_t> WebSocketClientConnectionObserver::ReadOpcodeAndPayloadLength(infra::DataInputStream& stream)
    {
        uint8_t opcode;
        uint32_t payloadLength;

        uint8_t payloadLength8;
        stream >> opcode >> payloadLength8;

        if (payloadLength8 == 126)
            payloadLength = infra::FromBigEndian(stream.Extract<uint16_t>());
        else
            payloadLength = payloadLength8;

        opcode &= 0xf;

        return std::make_pair(opcode, payloadLength);
    }

    void WebSocketClientConnectionObserver::Skip(infra::StreamReader& reader, std::size_t size)
    {
        while (size != 0)
        {
            auto range = reader.ExtractContiguousRange(size);
            size -= range.size();
        }
    }

    WebSocketClientConnectionObserver::FrameWriter::FrameWriter(infra::SharedPtr<infra::StreamWriter>&& writer, std::size_t sendSize)
        : infra::LimitedStreamWriter(*writer, sendSize)
        , writer(std::move(writer))
        , positionAtStart(this->writer->ConstructSaveMarker())
    {}

    WebSocketClientConnectionObserver::FrameWriter::~FrameWriter()
    {
        auto dataLength = this->writer->ConstructSaveMarker() - positionAtStart;
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        infra::SavedMarkerDataStream savedStream(stream, positionAtStart);

        savedStream << static_cast<uint8_t>(static_cast<uint8_t>(WebSocketOpCode::opCodeBin) | static_cast<uint8_t>(WebSocketMask::finMask));

        if (dataLength <= 125)
            savedStream << static_cast<uint8_t>(static_cast<uint8_t>(dataLength) | static_cast<uint8_t>(WebSocketMask::payloadMask));
        else
            savedStream << static_cast<uint8_t>(static_cast<uint8_t>(126) | static_cast<uint8_t>(WebSocketMask::payloadMask)) << infra::ToBigEndian(static_cast<uint16_t>(dataLength));

        savedStream << WebSocketMaskingKey();
    }

    WebSocketClientConnectionObserver::FrameReader::FrameReader(WebSocketClientConnectionObserver& client)
        : client(client)
        , reader(client.ConnectionObserver::Subject().ReceiveStream())
        , availableInCurrentFrame(client.availableInCurrentFrame)
    {}

    void WebSocketClientConnectionObserver::FrameReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        auto available = Available();
        errorPolicy.ReportResult(available >= range.size());
        range.shrink_from_back_to(available);

        while (!range.empty())
            ReadChunk(range);
    }

    uint8_t WebSocketClientConnectionObserver::FrameReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        auto empty = Empty();
        errorPolicy.ReportResult(!empty);

        if (!empty)
        {
            auto range = PeekContiguousRange(0);
            return range.front();
        }
        else
            return 0;
    }

    infra::ConstByteRange WebSocketClientConnectionObserver::FrameReader::ExtractContiguousRange(std::size_t max)
    {
        if (availableInCurrentFrame == 0 && !Empty())
            DiscoverNextFrame();

        auto range = reader->ExtractContiguousRange(std::min(max, availableInCurrentFrame));
        offset += range.size();
        availableInCurrentFrame -= range.size();

        return range;
    }

    infra::ConstByteRange WebSocketClientConnectionObserver::FrameReader::PeekContiguousRange(std::size_t start)
    {
        assert(start <= Available());
        auto save = ConstructSaveMarker();

        Rewind(save + start);
        auto range = ExtractContiguousRange(std::numeric_limits<std::size_t>::max());

        Rewind(save);
        return range;
    }

    bool WebSocketClientConnectionObserver::FrameReader::Empty() const
    {
        return Available() == 0;
    }

    std::size_t WebSocketClientConnectionObserver::FrameReader::Available() const
    {
        return client.unackedReadAvailable - client.skipDiscoveryPayload - offset;
    }

    std::size_t WebSocketClientConnectionObserver::FrameReader::ConstructSaveMarker() const
    {
        return offset;
    }

    void WebSocketClientConnectionObserver::FrameReader::Rewind(std::size_t marker)
    {
        reader->Rewind(0);
        availableInCurrentFrame = client.availableInCurrentFrame;
        offset = 0;

        Forward(marker);
    }

    void WebSocketClientConnectionObserver::FrameReader::AckReceived()
    {
        client.unackedReadAvailable -= offset;
        offset = 0;

        client.availableInCurrentFrame = availableInCurrentFrame;
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> WebSocketClientConnectionObserver::FrameReader::Reader()
    {
        return reader;
    }

    void WebSocketClientConnectionObserver::FrameReader::ReadChunk(infra::ByteRange& range)
    {
        auto read = reader->ExtractContiguousRange(std::min(range.size(), availableInCurrentFrame));
        infra::Copy(read, infra::Head(range, read.size()));
        range.pop_front(read.size());
        offset += read.size();
        availableInCurrentFrame -= read.size();

        if (availableInCurrentFrame == 0 && !Empty())
            DiscoverNextFrame();
    }

    void WebSocketClientConnectionObserver::FrameReader::DiscoverNextFrame()
    {
        while (availableInCurrentFrame == 0)
        {
            infra::DataInputStream::WithErrorPolicy stream(*reader);
            uint8_t opcode;
            uint32_t payloadLength;
            std::tie(opcode, payloadLength) = ReadOpcodeAndPayloadLength(stream);

            if (static_cast<WebSocketOpCode>(opcode) == WebSocketOpCode::opCodeBin)
                availableInCurrentFrame = payloadLength;
            else
                Skip(*reader, payloadLength);
        }
    }

    void WebSocketClientConnectionObserver::FrameReader::Forward(std::size_t amount)
    {
        while (amount != 0)
        {
            auto read = reader->ExtractContiguousRange(std::min(availableInCurrentFrame, amount));
            offset += read.size();
            availableInCurrentFrame -= read.size();
            amount -= read.size();

            if (availableInCurrentFrame == 0 && !Empty())
                DiscoverNextFrame();
        }
    }

    HttpClientWebSocketInitiation::HttpClientWebSocketInitiation(WebSocketClientObserverFactory& clientObserverFactory, HttpClientConnector& clientConnector,
        HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : HttpClientBasic(clientObserverFactory.Url(), clientObserverFactory.Port(), clientConnector)
        , result(result)
    {
        std::array<uint8_t, 16> randomData;
        randomDataGenerator.GenerateRandomData(randomData);
        infra::StringOutputStream::WithStorage<32> stream;
        stream << infra::AsBase64(randomData);

        headers.emplace_back("Upgrade", "websocket");
        headers.emplace_back("Connection", "Upgrade");
        headers.emplace_back("Sec-WebSocket-Key", stream.Storage());
        headers.emplace_back("Sec-WebSocket-Version", "13");
    }

    void HttpClientWebSocketInitiation::Stop(const infra::Function<void()>& onDone)
    {
        Cancel(onDone);
    }

    void HttpClientWebSocketInitiation::Connected()
    {
        Subject().Get(Path(), Headers());
    }

    services::HttpHeaders HttpClientWebSocketInitiation::Headers() const
    {
        return infra::MakeRange(headers);
    }

    void HttpClientWebSocketInitiation::StatusAvailable(HttpStatusCode statusCode)
    {
        if (statusCode != HttpStatusCode::SwitchingProtocols)
            ContentError();
    }

    void HttpClientWebSocketInitiation::HeaderAvailable(HttpHeader header)
    {
        if (header.Field() == "Upgrade")
        {
            if (header.Value() != "websocket")
                ContentError();
        }

        if (header.Field() == "Connection")
        {
            if (header.Value() != "Upgrade")
                ContentError();
        }

        if (header.Field() == "Sec-WebSocket-Version")
        {
            if (header.Value() != "13")
                ContentError();
        }
    }

    void HttpClientWebSocketInitiation::BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader)
    {}

    void HttpClientWebSocketInitiation::BodyComplete()
    {
        result.WebSocketInitiationDone();
    }

    void HttpClientWebSocketInitiation::Done()
    {
        // Done is called when the connection is expired; but for websockets, the connection must remain open. So instead, BodyComplete is used.
    }

    void HttpClientWebSocketInitiation::Error(bool intermittentFailure)
    {
        result.WebSocketInitiationError(intermittentFailure);
    }

    WebSocketClientInitiation::WebSocketClientInitiation(WebSocketClientObserverFactory& clientObserverFactory, ConnectionFactoryWithNameResolver& connectionFactory,
        WebSocketClientInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator, const Creators& creators)
        : clientObserverFactory(clientObserverFactory)
        , connectionFactory(connectionFactory)
        , result(result)
        , randomDataGenerator(randomDataGenerator)
        , initiationClient(creators.httpClientInitiationCreator, clientObserverFactory, *this, *this, randomDataGenerator)
    {}

    void WebSocketClientInitiation::CancelConnect()
    {
        if (httpClient)
        {
            httpClient.OnAllocatable([this]() { ExpiredWithCancelled(); });
            initiationClient->Stop(infra::emptyFunction);
        }
        else
            initiationClient->Stop([this]() { ExpiredWithCancelled(); });
    }

    WebSocketClientObserverFactory& WebSocketClientInitiation::Factory()
    {
        return clientObserverFactory;
    }

    void WebSocketClientInitiation::Connect(services::HttpClientObserverFactory& factory)
    {
        assert(httpClientObserverFactory == nullptr);
        httpClientObserverFactory = &factory;
        connectionFactory.Connect(*this);
    }

    void WebSocketClientInitiation::CancelConnect(services::HttpClientObserverFactory& factory)
    {
        if (httpClientObserverFactory != nullptr)
        {
            httpClientObserverFactory = nullptr;
            connectionFactory.CancelConnect(*this);
            result.InitiationCancelled();
        }
        else
        {
            httpClient.OnAllocatable([this]() { ExpiredWithCancelled(); });
            httpClient->Abort();
        }
    }

    infra::BoundedConstString WebSocketClientInitiation::Hostname() const
    {
        return HostFromUrl(clientObserverFactory.Url());
    }

    uint16_t WebSocketClientInitiation::Port() const
    {
        return clientObserverFactory.Port();
    }

    void WebSocketClientInitiation::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(httpClientObserverFactory != nullptr);
        httpClientObserverFactory->ConnectionEstablished([this, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
        {
            if (observer)
            {
                auto httpClientPtr = httpClient.Emplace(clientObserverFactory.Url());
                httpClient->AttachObserver(observer);
                createdObserver(httpClientPtr);
            }
        });

        httpClientObserverFactory = nullptr;
    }

    void WebSocketClientInitiation::ConnectionFailed(ConnectFailReason reason)
    {
        assert(httpClientObserverFactory != nullptr);

        switch (reason)
        {
            case ConnectFailReason::refused:
                httpClientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::refused);
                break;
            case ConnectFailReason::connectionAllocationFailed:
                httpClientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                break;
            case ConnectFailReason::nameLookupFailed:
                httpClientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
                break;
            default:
                std::abort();
        }

        httpClient.OnAllocatable([this, reason]() { ExpiredWithConnectionFailed(reason); });
        httpClientObserverFactory = nullptr;
    }

    void WebSocketClientInitiation::WebSocketInitiationDone()
    {
        result.InitiationDone(httpClient->ConnectionObserver::Subject());
        httpClient.OnAllocatable([this]() { ExpiredWithSuccess(); });
    }

    void WebSocketClientInitiation::WebSocketInitiationError(bool intermittentFailure)
    {
        httpClient.OnAllocatable([this]() { ExpiredWithError(); });
    }

    void WebSocketClientInitiation::ExpiredWithSuccess()
    {
        result.InitiationExpired();
    }

    void WebSocketClientInitiation::ExpiredWithError()
    {
        result.InitiationError();
    }

    void WebSocketClientInitiation::ExpiredWithConnectionFailed(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
    {
        result.InitiationConnectionFailed(reason);
    }

    void WebSocketClientInitiation::ExpiredWithCancelled()
    {
        result.InitiationCancelled();
    }

    WebSocketClientFactorySingleConnection::WebSocketClientFactorySingleConnection(ConnectionFactoryWithNameResolver& connectionFactory,
        hal::SynchronousRandomDataGenerator& randomDataGenerator, const WebSocketClientInitiation::Creators& creators)
        : connectionFactory(connectionFactory)
        , randomDataGenerator(randomDataGenerator)
        , creators(creators)
    {}

    void WebSocketClientFactorySingleConnection::Connect(WebSocketClientObserverFactory& factory)
    {
        initiation.Emplace(factory, connectionFactory, static_cast<WebSocketClientInitiationResult&>(*this), randomDataGenerator, creators);
    }

    void WebSocketClientFactorySingleConnection::CancelConnect(WebSocketClientObserverFactory& factory)
    {
        initiation->CancelConnect();
    }

    void WebSocketClientFactorySingleConnection::InitiationDone(services::Connection& connection)
    {
        auto& factory = initiation->Factory();
        auto webSocketConnection = webSocket.Emplace(PathFromUrl(factory.Url()), connection);
        initiation->initiationClient->Detach();
        connection.SwitchObserver(webSocketConnection);
        factory.ConnectionEstablished([webSocketConnection](infra::SharedPtr<ConnectionObserver> connectionObserver)
        {
            webSocketConnection->SetOwnership(nullptr, connectionObserver);
            connectionObserver->Attach(*webSocketConnection);
            connectionObserver->Connected();
        });
    }

    void WebSocketClientFactorySingleConnection::InitiationExpired()
    {
        initiation = infra::none;
    }

    void WebSocketClientFactorySingleConnection::InitiationError()
    {
        auto& factory = initiation->Factory();
        initiation = infra::none;
        factory.ConnectionFailed(WebSocketClientObserverFactory::ConnectFailReason::upgradeFailed);
    }

    void WebSocketClientFactorySingleConnection::InitiationConnectionFailed(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
    {
        auto& factory = initiation->Factory();
        initiation = infra::none;
        factory.ConnectionFailed(Translate(reason));
    }

    void WebSocketClientFactorySingleConnection::InitiationCancelled()
    {
        auto& factory = initiation->Factory();
        initiation = infra::none;
    }

    WebSocketClientObserverFactory::ConnectFailReason WebSocketClientFactorySingleConnection::Translate(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
    {
        switch (reason)
        {
            case ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::refused:
                return WebSocketClientObserverFactory::ConnectFailReason::refused;
            case ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::connectionAllocationFailed:
                return WebSocketClientObserverFactory::ConnectFailReason::connectionAllocationFailed;
            case ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::nameLookupFailed:
                return WebSocketClientObserverFactory::ConnectFailReason::nameLookupFailed;
            default:
                std::abort();
        }
    }
}
