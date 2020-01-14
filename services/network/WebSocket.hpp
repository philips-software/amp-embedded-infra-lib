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
        WebSocketFrameHeader() = default;
        explicit WebSocketFrameHeader(infra::DataInputStream& stream);

    public:
        bool IsValid() const;
        bool IsFinalFrame() const;

        WebSocketOpCode OpCode() const;
        uint64_t PayloadLength() const;
        const WebSocketMaskingKey& MaskingKey() const;

    private:
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
    protected:
        WebSocketObserverFactory() = default;
        WebSocketObserverFactory(const WebSocketObserverFactory& other) = delete;
        WebSocketObserverFactory& operator=(const WebSocketObserverFactory& other) = delete;
        ~WebSocketObserverFactory() = default;

    public:
        virtual void CreateWebSocketObserver(services::Connection& connection) = 0;
        virtual void CancelCreation() = 0;
    };

    class WebSocketObserverFactoryImpl
        : public WebSocketObserverFactory
    {
    public:
        struct Creators
        {
            infra::CreatorBase<services::ConnectionObserver, void()>& connectionCreator;
        };

        WebSocketObserverFactoryImpl(const Creators& creators);

        virtual void CreateWebSocketObserver(services::Connection& connection) override;
        virtual void CancelCreation() override;
        void Stop(const infra::Function<void()>& onDone);

    private:
        void OnAllocatable(services::Connection& connection);

    private:
        decltype(Creators::connectionCreator) connectionCreator;
        infra::NotifyingSharedOptional<infra::ProxyCreator<decltype(Creators::connectionCreator)>> webSocketConnectionObserver;
    };
}

namespace infra
{
    DataInputStream& operator>>(DataInputStream& stream, services::WebSocketFrameHeader& header);
    DataOutputStream& operator<<(DataOutputStream& stream, services::WebSocketOpCode opcode);
}

#endif
