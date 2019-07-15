#ifndef SERVICES_NETWORK_WEBSOCKET_HPP
#define SERVICES_NETWORK_WEBSOCKET_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include "services/network/Http.hpp"

namespace services
{
    enum class WebSocketMask : uint8_t
    {
        finMask = 0x80,
        rsvMask = 0x70,
        opCodeMask = 0x0F,
        payloadMask = 0x80,
        payloadLengthMask = 0x7F
    };

    enum class WebSocketOpCode : uint8_t
    {
        opCodeContinue = 0x00,
        opCodeText = 0x01,
        opCodeBin = 0x02,
        opCodeClose = 0x08,
        opCodePing = 0x09,
        opCodePong = 0x0A
    };

    using WebSocketMaskingKey = std::array<uint8_t, 4>;

    class WebSocketFrameHeader
    {
    public:
        explicit WebSocketFrameHeader(infra::StreamReader& reader);

    public:
        bool IsComplete() const;
        bool IsValid() const;
        bool IsFinalFrame() const;

        WebSocketOpCode OpCode() const;
        uint64_t PayloadLength() const;
        const WebSocketMaskingKey& MaskingKey() const;

    private:
        bool complete = false;
        bool finalFrame = false;
        bool masked = false;
        uint8_t rsv = 0;
        uint64_t payloadLength = 0;
        WebSocketOpCode opCode;
        WebSocketMaskingKey maskingKey;
    };

    class WebSocket
    {
    public:
        static void UpgradeHeaders(infra::BoundedVector<const services::HttpHeader>& headers, infra::BoundedConstString protocol);
    };

    class WebSocketObserverFactory
    {
    public:
        struct Creators
        {
            infra::CreatorBase<services::ConnectionObserver, void(services::Connection& connection, infra::BoundedConstString handshakeKey)>& connectionCreator;
        };

        WebSocketObserverFactory(const Creators& creators);

        virtual void CreateWebSocketObserver(services::Connection& connection, infra::BoundedConstString handshakeKey, services::IPAddress address);
        void CancelCreation();
        void Stop(const infra::Function<void()>& onDone);

    private:
        void OnAllocatable(services::Connection& connection);

    private:
        decltype(Creators::connectionCreator) connectionCreator;
        infra::NotifyingSharedOptional<infra::ProxyCreator<decltype(Creators::connectionCreator)>> webSocketConnectionObserver;
        infra::BoundedString::WithStorage<64> handshakeKey;
    };
}

namespace infra
{
    DataOutputStream& operator<<(DataOutputStream& stream, services::WebSocketOpCode opcode);
}

#endif
