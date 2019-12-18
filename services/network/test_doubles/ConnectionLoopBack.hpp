#ifndef NETWORK_CONNECTION_LOOP_BACK_HPP
#define NETWORK_CONNECTION_LOOP_BACK_HPP

#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include <map>

namespace services
{
    class ConnectionLoopBack;
    class ConnectionLoopBackFactory;

    class ConnectionLoopBackPeer
        : public Connection
    {
    public:
        ConnectionLoopBackPeer(ConnectionLoopBackPeer& peer, ConnectionLoopBack& loopBack);

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

        void SetOwnership(const infra::SharedPtr<ConnectionLoopBack>& owner, const infra::SharedPtr<ConnectionObserver>& observer);
        void ResetOwnership();

    private:
        void TryAllocateSendStream();

    private:
        class StreamWriterLoopBack
            : public infra::LimitedStreamWriter
        {
        public:
            explicit StreamWriterLoopBack(ConnectionLoopBackPeer& connection, std::size_t size);
            ~StreamWriterLoopBack();

        private:
            ConnectionLoopBackPeer& connection;
            std::size_t sent = 0;
            infra::StdVectorOutputStreamWriter::WithStorage vectorStreamWriter;
        };

        class StreamReaderLoopBack
            : public infra::BoundedDequeInputStreamReader
        {
        public:
            explicit StreamReaderLoopBack(ConnectionLoopBackPeer& connection);

            void ConsumeRead();

        private:
            ConnectionLoopBackPeer& connection;
        };

    private:
        ConnectionLoopBackPeer& peer;
        ConnectionLoopBack& loopBack;
        infra::BoundedDeque<uint8_t>::WithMaxSize<1024> sendBuffer;
        infra::SharedOptional<StreamWriterLoopBack> streamWriter;
        std::size_t requestedSendSize = 0;
        infra::SharedOptional<StreamReaderLoopBack> streamReader;
        infra::SharedPtr<ConnectionLoopBack> owner;
    };

    class ConnectionLoopBack
        : public infra::EnableSharedFromThis<ConnectionLoopBack>
    {
    public:
        ConnectionLoopBack();

        ConnectionLoopBackPeer& Server();
        ConnectionLoopBackPeer& Client();

        void Connect(infra::SharedPtr<services::ConnectionObserver> serverObserver, infra::SharedPtr<services::ConnectionObserver> clientObserver);

    private:
        ConnectionLoopBackPeer server;
        ConnectionLoopBackPeer client;
    };

    class ConnectionLoopBackListener
    {
    public:
        ConnectionLoopBackListener(uint16_t port, ConnectionLoopBackFactory& loopBackFactory, ServerConnectionObserverFactory& connectionObserverFactory);
        ~ConnectionLoopBackListener();

        void Accept(ClientConnectionObserverFactory& clientObserverFactory);

    private:
        uint16_t port;
        ConnectionLoopBackFactory& loopBackFactory;
        ServerConnectionObserverFactory& connectionObserverFactory;
    };

    class ConnectionLoopBackConnector
        : public infra::EnableSharedFromThis<ConnectionLoopBackConnector>
    {
    public:
        ConnectionLoopBackConnector(ConnectionLoopBackFactory& loopBackFactory, ClientConnectionObserverFactory& connectionObserverFactory);

        void Connect();

    private:
        ConnectionLoopBackFactory& loopBackFactory;
        ClientConnectionObserverFactory& connectionObserverFactory;
    };

    class ConnectionLoopBackFactory
        : public ConnectionFactory
    {
    public:
        ~ConnectionLoopBackFactory();

        void RegisterListener(uint16_t port, ConnectionLoopBackListener* listener);
        void UnregisterListener(uint16_t port);

        virtual infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions) override;
        virtual void Connect(ClientConnectionObserverFactory& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& factory) override;

    private:
        friend class ConnectionLoopBackConnector;
        std::map<uint16_t, ConnectionLoopBackListener*> listeners;
        std::vector<infra::SharedPtr<ConnectionLoopBackConnector>> connectors;
    };

}

#endif
