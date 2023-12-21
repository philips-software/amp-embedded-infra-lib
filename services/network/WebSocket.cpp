#include "services/network/WebSocket.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/EnumCast.hpp"

namespace
{
    const uint8_t extendedPayloadLength16 = 126;
    const uint8_t extendedPayloadLength64 = 127;
}

namespace services
{
    WebSocketFrameHeader::WebSocketFrameHeader(infra::DataInputStream& stream)
    {
        uint8_t first(0);
        uint8_t second(0);
        stream >> first >> second;
        finalFrame = (first & infra::enum_cast(WebSocketMask::finMask)) != 0;
        rsv = (first & infra::enum_cast(WebSocketMask::rsvMask)) >> 4;
        opCode = static_cast<WebSocketOpCode>(first & infra::enum_cast(WebSocketMask::opCodeMask));
        masked = (second & infra::enum_cast(WebSocketMask::payloadMask)) != 0;
        payloadLength = second & infra::enum_cast(WebSocketMask::payloadLengthMask);

        if (payloadLength == extendedPayloadLength16)
        {
            infra::BigEndian<uint16_t> extractedPayloadLength = 0;
            stream >> extractedPayloadLength;
            payloadLength = extractedPayloadLength;
        }
        else if (payloadLength == extendedPayloadLength64)
        {
            infra::BigEndian<uint64_t> extractedPayloadLength = 0;
            stream >> extractedPayloadLength;
            payloadLength = extractedPayloadLength;
        }

        stream >> maskingKey;
    }

    bool WebSocketFrameHeader::IsValid() const
    {
        if (rsv != 0)
            return false;
        if (opCode > WebSocketOpCode::opCodePong ||
            (opCode > WebSocketOpCode::opCodeBin && opCode < WebSocketOpCode::opCodeClose))
            return false;
        if (!masked)
            return false;

        return true;
    }

    bool WebSocketFrameHeader::IsFinalFrame() const
    {
        return finalFrame;
    }

    WebSocketOpCode WebSocketFrameHeader::OpCode() const
    {
        return opCode;
    }

    uint64_t WebSocketFrameHeader::PayloadLength() const
    {
        return payloadLength;
    }

    const WebSocketMaskingKey& WebSocketFrameHeader::MaskingKey() const
    {
        return maskingKey;
    }

    void WebSocket::UpgradeHeaders(infra::BoundedVector<const services::HttpHeader>& headers, infra::BoundedConstString protocol)
    {
        really_assert(headers.max_size() - headers.size() >= 5);

        headers.push_back(services::HttpHeader("Upgrade", "websocket"));
        headers.push_back(services::HttpHeader("Connection", "Upgrade"));
        headers.push_back(services::HttpHeader("Sec-Websocket-Key", "AQIDBAUGBbgJCgsMDQ4PEC=="));
        headers.push_back(services::HttpHeader("Sec-Websocket-Protocol", protocol));
        headers.push_back(services::HttpHeader("Sec-Websocket-Version", "13"));
    }

    WebSocketObserverFactoryImpl::WebSocketObserverFactoryImpl(const Creators& creators)
        : connectionCreator(creators.connectionCreator)
    {}

    void WebSocketObserverFactoryImpl::CreateWebSocketObserver(services::Connection& connection)
    {
        if (webSocketConnectionObserver.Allocatable())
            OnAllocatable(connection);
        else
        {
            webSocketConnectionObserver.OnAllocatable([this, &connection]()
                {
                    OnAllocatable(connection);
                });
            if (webSocketConnectionObserver)
                (*webSocketConnectionObserver)->Close();
        }
    }

    void WebSocketObserverFactoryImpl::CancelCreation()
    {
        webSocketConnectionObserver.OnAllocatable(nullptr);
    }

    void WebSocketObserverFactoryImpl::Stop(const infra::Function<void()>& onDone)
    {
        if (!webSocketConnectionObserver.Allocatable())
        {
            webSocketConnectionObserver.OnAllocatable(onDone);

            if (webSocketConnectionObserver)
                (*webSocketConnectionObserver)->Close();
        }
        else
            onDone();
    }

    void WebSocketObserverFactoryImpl::OnAllocatable(services::Connection& connection)
    {
        auto observer = webSocketConnectionObserver.Emplace(connectionCreator);
        connection.Detach();
        connection.Attach(infra::MakeContainedSharedObject(**observer, observer));
        webSocketConnectionObserver.OnAllocatable(nullptr);
    }
}

namespace infra
{
    DataInputStream& operator>>(DataInputStream& stream, services::WebSocketFrameHeader& header)
    {
        header = services::WebSocketFrameHeader(stream);
        return stream;
    }

    DataOutputStream& operator<<(DataOutputStream& stream, services::WebSocketOpCode opcode)
    {
        stream << static_cast<uint8_t>(enum_cast(services::WebSocketMask::finMask) | (enum_cast(opcode) & enum_cast(services::WebSocketMask::opCodeMask)));
        return stream;
    }
}
