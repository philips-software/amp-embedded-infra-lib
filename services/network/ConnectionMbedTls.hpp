#ifndef SERVICES_CONNECTION_MBED_TLS_HPP
#define SERVICES_CONNECTION_MBED_TLS_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/SharedOptional.hpp"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cache.h"
#include "services/network/CertificatesMbedTls.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    class ConnectionMbedTls
        : public ConnectionWithHostname
        , public ConnectionObserver
        , public infra::EnableSharedFromThis<ConnectionMbedTls>
    {
    public:
        struct ServerParameters
        {
            mbedtls_ssl_cache_context& serverCache;
            bool clientAuthenticationNeeded;
        };

        struct ClientParameters
        {
            mbedtls_ssl_session& clientSession;
        };

        using Parameters = infra::Variant<ServerParameters, ClientParameters>;
        struct ParametersWorkaround
        {
            Parameters parameters;
        };

        ConnectionMbedTls(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const ParametersWorkaround& parameters);
        ConnectionMbedTls(const ConnectionMbedTls& other) = delete;
        ~ConnectionMbedTls();

        void CreatedObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;
        virtual void Close() override;
        virtual void Abort() override;

        // Implementation of ConnectionWithHostname
        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;
        virtual void SetHostname(infra::BoundedConstString hostname) override;

        virtual void TlsInitFailure(int reason);
        virtual void TlsReadFailure(int reason);
        virtual void TlsWriteFailure(int reason);
        virtual void TlsLog(int level, const char* file, int line, const char* message);

    private:
        void InitTls();
        void TryAllocateSendStream();
        void TryAllocateEncryptedSendStream();
        static int StaticSslSend(void* context, const unsigned char* buffer, std::size_t size);
        static int StaticSslReceive(void *context, unsigned char* buffer, size_t size);
        int SslSend(infra::ConstByteRange buffer);
        int SslReceive(infra::ByteRange buffer);
        void TrySend();
        static int StaticGenerateRandomData(void* data, unsigned char* output, std::size_t size);
        void GenerateRandomData(infra::ByteRange data);
        static void StaticDebugWrapper(void* context, int level, const char* file, int line, const char* message);

    private:
        class StreamWriterMbedTls
            : public infra::BoundedVectorStreamWriter
        {
        public:
            explicit StreamWriterMbedTls(ConnectionMbedTls& connection);
            ~StreamWriterMbedTls();

        private:
            ConnectionMbedTls& connection;
        };

        class StreamReaderMbedTls
            : public infra::BoundedDequeInputStreamReader
        {
        public:
            explicit StreamReaderMbedTls(ConnectionMbedTls& connection);

            void ConsumeRead();

        private:
            ConnectionMbedTls& connection;
        };

    private:
        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        bool server;
        mbedtls_ssl_session* clientSession = nullptr;
        mbedtls_ssl_context sslContext;
        mbedtls_ssl_config sslConfig;
        mbedtls_ctr_drbg_context ctr_drbg;

        infra::BoundedDeque<uint8_t>::WithMaxSize<1024> receiveBuffer;
        infra::BoundedVector<uint8_t>::WithMaxSize<1024> sendBuffer;
        bool sending = false;

        infra::SharedOptional<StreamWriterMbedTls> streamWriter;
        std::size_t requestedSendSize = 0;
        bool flushScheduled = false;
        infra::SharedOptional<StreamReaderMbedTls> receiveReader;

        infra::SharedPtr<infra::StreamWriter> encryptedSendWriter;
        std::size_t encryptedSendStreamSize = 0;

        bool initialHandshake = true;
    };

    using AllocatorConnectionMbedTls = infra::SharedObjectAllocator<ConnectionMbedTls,
        void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters)>;

    class ConnectionMbedTlsListener
        : public ServerConnectionObserverFactory
    {
    public:
        ConnectionMbedTlsListener(AllocatorConnectionMbedTls& allocator, ServerConnectionObserverFactory& factory,
            CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, mbedtls_ssl_cache_context& serverCache, bool clientAuthenticationNeeded);

        virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address) override;

        void SetListener(infra::SharedPtr<void> listener);

    private:
        AllocatorConnectionMbedTls& allocator;
        ServerConnectionObserverFactory& factory;
        CertificatesMbedTls& certificates;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        mbedtls_ssl_cache_context& serverCache;
        bool clientAuthenticationNeeded;
        infra::SharedPtr<void> listener;
    };

    using AllocatorConnectionMbedTlsListener = infra::SharedObjectAllocator<ConnectionMbedTlsListener,
        void(AllocatorConnectionMbedTls& allocator, ServerConnectionObserverFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, mbedtls_ssl_cache_context& serverCache, bool clientAuthenticationNeeded)>;

    class ConnectionFactoryMbedTls;

    class ConnectionMbedTlsConnector
        : public ClientConnectionObserverFactory
    {
    public:
        ConnectionMbedTlsConnector(ConnectionFactoryMbedTls& factory, ConnectionFactory& networkFactory, ClientConnectionObserverFactory& clientFactory);

        virtual IPAddress Address() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    private:
        friend class ConnectionFactoryMbedTls;

        ConnectionFactoryMbedTls& factory;
        ClientConnectionObserverFactory& clientFactory;
    };

#ifdef _MSC_VER                                                                                                         //TICS !POR#021
#pragma warning(disable:4503)                                                                                           //TICS !POR#018
#endif

    class ConnectionFactoryMbedTls
        : public ConnectionFactory
    {
    public:
        template<std::size_t MaxConnections, std::size_t MaxListeners, std::size_t MaxConnectors>
            using WithMaxConnectionsListenersAndConnectors = infra::WithStorage<infra::WithStorage<infra::WithStorage<ConnectionFactoryMbedTls
                , AllocatorConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>
                , AllocatorConnectionMbedTlsListener::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>
                , infra::BoundedList<ConnectionMbedTlsConnector>::WithMaxSize<MaxConnectors>>;

        ConnectionFactoryMbedTls(AllocatorConnectionMbedTls& connectionAllocator, AllocatorConnectionMbedTlsListener& listenerAllocator, infra::BoundedList<ConnectionMbedTlsConnector>& connectors,
            ConnectionFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, bool needsAuthenticationDefault = false);
        ~ConnectionFactoryMbedTls();

        virtual infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& connectionObserverFactory, IPVersions versions = IPVersions::both) override;
        virtual void Connect(ClientConnectionObserverFactory& connectionObserverFactory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& connectionObserverFactory) override;

        infra::SharedPtr<ConnectionMbedTls> Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address);
        void Remove(ConnectionMbedTlsConnector& connector);

    protected:
        virtual bool NeedsAuthentication(uint16_t port) const;

    private:
        void TryConnect();

    private:
        AllocatorConnectionMbedTls& connectionAllocator;
        AllocatorConnectionMbedTlsListener& listenerAllocator;
        infra::IntrusiveList<ClientConnectionObserverFactory> waitingConnects;
        infra::BoundedList<ConnectionMbedTlsConnector>& connectors;
        ConnectionFactory& factory;
        CertificatesMbedTls& certificates;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        mbedtls_ssl_cache_context serverCache;
        mbedtls_ssl_session clientSession = {};
        IPAddress previousAddress;
        bool needsAuthenticationDefault;
    };

    class ConnectionFactoryWithNameResolverForTls
        : public ConnectionFactoryWithNameResolver
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        ConnectionFactoryWithNameResolverForTls(ConnectionFactoryWithNameResolver& connectionFactoryWithNameResolver);

        // Implementation of ConnectionFactoryWithNameResolver
        virtual void Connect(ClientConnectionObserverFactoryWithNameResolver& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory) override;

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    private:
        void TryConnect();

    private:
        ConnectionFactoryWithNameResolver& connectionFactoryWithNameResolver;
        ClientConnectionObserverFactoryWithNameResolver* clientConnectionFactory = nullptr;
        infra::IntrusiveList<ClientConnectionObserverFactoryWithNameResolver> waitingConnects;
    };
}

#endif
