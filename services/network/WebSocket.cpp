#include "WebSocket.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Endian.hpp"

namespace
{
    const uint8_t extendedPayloadLength16 = 126;
    const uint8_t extendedPayloadLength64 = 127;
}

template<typename T>
constexpr auto enum_cast(T t) -> typename std::underlying_type<T>::type
{
    return static_cast<typename std::underlying_type<T>::type>(t);
}

namespace services
{
    WebSocketFrameHeader::WebSocketFrameHeader(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy data(reader, infra::softFail);

        uint8_t first(0);
        uint8_t second(0);
        data >> first >> second;
        finalFrame = (first & enum_cast(WebSocketMask::finMask)) != 0;
        rsv = (first & enum_cast(WebSocketMask::rsvMask)) >> 4;
        opCode = static_cast<WebSocketOpCode>(first & enum_cast(WebSocketMask::opCodeMask));
        masked = (second & enum_cast(WebSocketMask::payloadMask)) != 0;
        payloadLength = second & enum_cast(WebSocketMask::payloadLengthMask);

        if (payloadLength == extendedPayloadLength16)
        {
            infra::BigEndian<uint16_t> extractedPayloadLength = 0;
            data >> extractedPayloadLength;
            payloadLength = extractedPayloadLength;
        }
        else if (payloadLength == extendedPayloadLength64)
        {
            infra::BigEndian<uint64_t> extractedPayloadLength = 0;
            data >> extractedPayloadLength;
            payloadLength = extractedPayloadLength;
        }

        data >> maskingKey;

        if (!data.Failed())
            complete = true;
    }

    bool WebSocketFrameHeader::IsComplete() const
    {
        return complete;
    }

    bool WebSocketFrameHeader::IsValid() const
    {
        if (!IsComplete())
            return false;
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
}

namespace infra
{
    DataOutputStream& operator<<(DataOutputStream& stream, services::WebSocketOpCode opcode)
    {
        stream << static_cast<uint8_t>(enum_cast(services::WebSocketMask::finMask) | (enum_cast(opcode) & enum_cast(services::WebSocketMask::opCodeMask)));
        return stream;
    }
}
