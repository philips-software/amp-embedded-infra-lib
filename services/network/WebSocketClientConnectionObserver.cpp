#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/Endian.hpp"
#include "services/network/WebSocket.hpp"
#include <cassert>

namespace services
{
    WebSocketClientConnectionObserver::WebSocketClientConnectionObserver()
        : streamWriter([this]()
              {
                  StreamWriterAllocatable();
              })
    {}

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
            Connection::Observer().SendStreamAvailable(streamWriter.Emplace(std::move(writer), *infra::PostAssign(requestedSendSize, infra::none)));
        }
    }

    void WebSocketClientConnectionObserver::DataReceived()
    {
        auto before = unackedReadAvailable;
        DiscoverData();
        TryAllocateSendStream();

        if (before != unackedReadAvailable)
            Observer().DataReceived();
    }

    void WebSocketClientConnectionObserver::Detaching()
    {
        Connection::Detach();
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
        : HttpClientWebSocketInitiation(clientObserverFactory, clientConnector, result, randomDataGenerator, noAutoConnect)
    {
        Connect();
    }

    HttpClientWebSocketInitiation::HttpClientWebSocketInitiation(WebSocketClientObserverFactory& clientObserverFactory, HttpClientConnector& clientConnector,
        HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator, NoAutoConnect)
        : HttpClientBasic(clientObserverFactory.Url(), clientObserverFactory.Port(), clientConnector, noAutoConnect)
        , result(result)
    {
        std::array<uint8_t, 16> randomData;
        randomDataGenerator.GenerateRandomData(randomData);
        infra::TextOutputStream::WithWriter<infra::StringOutputStreamWriter> stream(webSocketKey);
        stream << infra::AsBase64(randomData);

        headers.emplace_back("Content-Length", "0");
        headers.emplace_back("Upgrade", "websocket");
        headers.emplace_back("Connection", "Upgrade");
        headers.emplace_back("Sec-WebSocket-Key", webSocketKey);
        headers.emplace_back("Sec-WebSocket-Version", "13");
    }

    void HttpClientWebSocketInitiation::Stop(const infra::Function<void()>& onDone)
    {
        Cancel(onDone);
    }

    void HttpClientWebSocketInitiation::Attached()
    {
        HttpClientBasic::Attached();
        Subject().Get(Path(), Headers());
    }

    services::HttpHeaders HttpClientWebSocketInitiation::Headers() const
    {
        return infra::MakeRange(headers);
    }

    void HttpClientWebSocketInitiation::StatusAvailable(HttpStatusCode statusCode)
    {
        HttpClientBasic::StatusAvailable(statusCode);

        if (statusCode != HttpStatusCode::SwitchingProtocols)
            ContentError();
    }

    void HttpClientWebSocketInitiation::HeaderAvailable(HttpHeader header)
    {
        if (infra::CaseInsensitiveCompare(header.Field(), "Upgrade"))
        {
            if (header.Value() != "websocket")
                ContentError();
        }

        if (infra::CaseInsensitiveCompare(header.Field(), "Connection"))
        {
            if (!infra::CaseInsensitiveCompare(header.Value(), "Upgrade"))
                ContentError();
        }

        if (infra::CaseInsensitiveCompare(header.Field(), "Sec-WebSocket-Version"))
        {
            if (header.Value() != "13")
                ContentError();
        }
    }

    void HttpClientWebSocketInitiation::BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader)
    {}

    void HttpClientWebSocketInitiation::BodyComplete()
    {
        // Switching http client off will result in Detaching(), resulting in CloseConnection() being called
        done = true;
        HttpClientBasic::BodyComplete();
        result.WebSocketInitiationDone(Subject().GetConnection());
    }

    void HttpClientWebSocketInitiation::Done()
    {
        // Done is called when the connection is expired; but for websockets, the connection must remain open. So instead, BodyComplete is used.
    }

    void HttpClientWebSocketInitiation::Error(bool intermittentFailure)
    {
        result.WebSocketInitiationError(initiationError);
    }

    void HttpClientWebSocketInitiation::CloseConnection()
    {
        // Default behaviour is to close the connection after the request is done. But successful
        // opening of the websockt should result in the connection being kept open.
        if (!done)
            HttpClientBasic::CloseConnection();
    }

    void HttpClientWebSocketInitiation::ConnectionFailed(HttpClientObserverFactory::ConnectFailReason reason)
    {
        initiationError = Translate(reason);
        HttpClientBasic::ConnectionFailed(reason);
    }

    WebSocketClientObserverFactory::ConnectFailReason HttpClientWebSocketInitiation::Translate(HttpClientObserverFactory::ConnectFailReason reason) const
    {
        switch (reason)
        {
            case HttpClientObserverFactory::ConnectFailReason::refused:
                return WebSocketClientObserverFactory::ConnectFailReason::refused;
            case HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed:
                return WebSocketClientObserverFactory::ConnectFailReason::connectionAllocationFailed;
            case HttpClientObserverFactory::ConnectFailReason::nameLookupFailed:
                return WebSocketClientObserverFactory::ConnectFailReason::nameLookupFailed;
            default:
                std::abort();
        }
    }

    WebSocketClientFactorySingleConnection::WebSocketClientFactorySingleConnection(hal::SynchronousRandomDataGenerator& randomDataGenerator, const Creators& creators)
        : randomDataGenerator(randomDataGenerator)
        , creators(creators)
    {}

    void WebSocketClientFactorySingleConnection::Stop(const infra::Function<void()>& onDone)
    {
        if (!webSocket.Allocatable())
        {
            webSocket.OnAllocatable(onDone);
            if (webSocket)
                webSocket->Abort();
        }
        else
            onDone();
    }

    void WebSocketClientFactorySingleConnection::Connect(WebSocketClientObserverFactory& factory)
    {
        initiation.emplace(factory, static_cast<WebSocketClientInitiationResult&>(*this), randomDataGenerator, creators);
    }

    void WebSocketClientFactorySingleConnection::CancelConnect(WebSocketClientObserverFactory& factory, const infra::Function<void()>& onDone)
    {
        initiation->CancelConnect(onDone);
    }

    void WebSocketClientFactorySingleConnection::InitiationDone(services::Connection& connection)
    {
        auto& factory = initiation->Factory();
        auto webSocketConnection = webSocket.Emplace();

        connection.Detach();
        connection.Attach(webSocketConnection);

        initiation = infra::none;
        factory.ConnectionEstablished([webSocketConnection](infra::SharedPtr<ConnectionObserver> connectionObserver)
            {
                webSocketConnection->Attach(connectionObserver);
            });
    }

    void WebSocketClientFactorySingleConnection::InitiationError(WebSocketClientObserverFactory::ConnectFailReason reason)
    {
        auto& factory = initiation->Factory();
        initiation = infra::none;
        factory.ConnectionFailed(reason);
    }

    void WebSocketClientFactorySingleConnection::InitiationCancelled()
    {
        initiation = infra::none;
    }

    WebSocketClientFactorySingleConnection::WebSocketClientInitiation::WebSocketClientInitiation(WebSocketClientObserverFactory& clientObserverFactory,
        WebSocketClientInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator, const Creators& creators)
        : clientObserverFactory(clientObserverFactory)
        , result(result)
        , initiationClient(creators.httpClientInitiationCreator, clientObserverFactory, *this, randomDataGenerator)
    {}

    void WebSocketClientFactorySingleConnection::WebSocketClientInitiation::CancelConnect(const infra::Function<void()>& onDone)
    {
        onStopped = onDone;
        initiationClient->Stop([this]()
            {
                auto onDone = onStopped;
                result.InitiationCancelled();
                onDone();
            });
    }

    WebSocketClientObserverFactory& WebSocketClientFactorySingleConnection::WebSocketClientInitiation::Factory()
    {
        return clientObserverFactory;
    }

    void WebSocketClientFactorySingleConnection::WebSocketClientInitiation::WebSocketInitiationDone(Connection& connection)
    {
        result.InitiationDone(connection);
    }

    void WebSocketClientFactorySingleConnection::WebSocketClientInitiation::WebSocketInitiationError(WebSocketClientObserverFactory::ConnectFailReason reason)
    {
        result.InitiationError(reason);
    }
}
