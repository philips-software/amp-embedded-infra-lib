#ifndef SERVICES_CONNECTION_MBED_TLS_HPP
#define SERVICES_CONNECTION_MBED_TLS_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
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
        enum class CertificateValidation : uint8_t
        {
            Default,
            Enabled,
            Disabled
        };

        struct ServerParameters
        {
            mbedtls_ssl_cache_context& serverCache;
            CertificateValidation certificateValidation;
        };

        struct ClientParameters
        {
            mbedtls_ssl_session& clientSession;
            bool& clientSessionObtained;
            CertificateValidation certificateValidation;
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
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Attached() override;
        void Detaching() override;
        void Close() override;
        void Abort() override;

        // Implementation of ConnectionWithHostname
        void RequestSendStream(std::size_t sendSize) override;
        std::size_t MaxSendStreamSize() const override;
        infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        void AckReceived() override;
        void CloseAndDestroy() override;
        void AbortAndDestroy() override;
        void SetHostname(infra::BoundedConstString hostname) override;

        virtual void TlsInitFailure(int reason);
        virtual void TlsReadFailure(int reason);
        virtual void TlsWriteFailure(int reason);
        virtual void TlsLog(int level, const char* file, int line, const char* message);

    private:
        void InitTls();
        int GetAuthMode(const ParametersWorkaround& parameters) const;
        void TryAllocateSendStream();
        void TryAllocateEncryptedSendStream();
        static int StaticSslSend(void* context, const unsigned char* buffer, std::size_t size);
        static int StaticSslReceive(void* context, unsigned char* buffer, size_t size);
        int SslSend(infra::ConstByteRange buffer);
        int SslReceive(infra::ByteRange buffer);
        void TrySend();
        static int StaticGenerateRandomData(void* data, unsigned char* output, std::size_t size);
        void GenerateRandomData(infra::ByteRange data);
        static void StaticDebugWrapper(void* context, int level, const char* file, int line, const char* message);

    private:
        class StreamWriterMbedTls
            : public infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>
        {
        public:
            explicit StreamWriterMbedTls(ConnectionMbedTls& connection, uint32_t size);
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
        bool* clientSessionObtained = nullptr;
        mbedtls_ssl_context sslContext;
        mbedtls_ssl_config sslConfig;
        mbedtls_ctr_drbg_context ctr_drbg;

        infra::BoundedDeque<uint8_t>::WithMaxSize<1024> receiveBuffer;
        infra::BoundedVector<uint8_t>::WithMaxSize<1024> sendBuffer;
        infra::BoundedString::WithStorage<MBEDTLS_SSL_MAX_HOST_NAME_LEN + 1> terminatedHostname;
        bool sending = false;

        infra::SharedOptional<StreamWriterMbedTls> streamWriter;
        std::size_t requestedSendSize = 0;
        bool dataReceivedScheduled = false;
        bool flushScheduled = false;
        infra::NotifyingSharedOptional<StreamReaderMbedTls> receiveReader;
        infra::SharedPtr<void> keepAliveForReader;

        infra::SharedPtr<infra::StreamWriter> encryptedSendWriter;
        std::size_t encryptedSendStreamSize = 0;

        bool initialHandshake = true;
        bool closing = false;
        bool aborting = false;
        bool destructed = false;
    };

    using AllocatorConnectionMbedTls = infra::SharedObjectAllocator<ConnectionMbedTls,
        void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const ConnectionMbedTls::ParametersWorkaround& parameters)>;

    class ConnectionMbedTlsListener
        : public ServerConnectionObserverFactory
    {
    public:
        ConnectionMbedTlsListener(AllocatorConnectionMbedTls& allocator, ServerConnectionObserverFactory& factory,
            CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, mbedtls_ssl_cache_context& serverCache, ConnectionMbedTls::CertificateValidation certificateValidation);

        void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address) override;

        void SetListener(infra::SharedPtr<void> listener);

    private:
        AllocatorConnectionMbedTls& allocator;
        ServerConnectionObserverFactory& factory;
        CertificatesMbedTls& certificates;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        mbedtls_ssl_cache_context& serverCache;
        ConnectionMbedTls::CertificateValidation certificateValidation;
        infra::SharedPtr<void> listener;
    };

    using AllocatorConnectionMbedTlsListener = infra::SharedObjectAllocator<ConnectionMbedTlsListener,
        void(AllocatorConnectionMbedTls& allocator, ServerConnectionObserverFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, mbedtls_ssl_cache_context& serverCache, ConnectionMbedTls::CertificateValidation certificateValidation)>;

    class ConnectionFactoryMbedTls;

    class ConnectionMbedTlsConnector
        : public ClientConnectionObserverFactory
    {
    public:
        ConnectionMbedTlsConnector(ConnectionFactoryMbedTls& factory, ConnectionFactory& networkFactory, ClientConnectionObserverFactory& clientFactory);

        IPAddress Address() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    private:
        void CancelConnect();

    private:
        friend class ConnectionFactoryMbedTls;

        ConnectionFactoryMbedTls& factory;
        ConnectionFactory& networkFactory;
        ClientConnectionObserverFactory& clientFactory;
    };

    class ConnectionFactoryWithNameResolverMbedTls;

    class ConnectionMbedTlsConnectorWithNameResolver
        : public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        ConnectionMbedTlsConnectorWithNameResolver(ConnectionFactoryWithNameResolverMbedTls& factory, ConnectionFactoryWithNameResolver& networkFactory, ClientConnectionObserverFactoryWithNameResolver& clientFactory);

        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    private:
        void CancelConnect();

    private:
        friend class ConnectionFactoryWithNameResolverMbedTls;

        ConnectionFactoryWithNameResolverMbedTls& factory;
        ConnectionFactoryWithNameResolver& networkFactory;
        ClientConnectionObserverFactoryWithNameResolver& clientFactory;
    };

#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

    class ConnectionFactoryMbedTls
        : public ConnectionFactory
    {
    public:
        template<std::size_t MaxConnections, std::size_t MaxListeners, std::size_t MaxConnectors>
        using WithMaxConnectionsListenersAndConnectors = infra::WithStorage<infra::WithStorage<infra::WithStorage<ConnectionFactoryMbedTls, AllocatorConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>, AllocatorConnectionMbedTlsListener::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>, infra::BoundedList<ConnectionMbedTlsConnector>::WithMaxSize<MaxConnectors>>;

        ConnectionFactoryMbedTls(AllocatorConnectionMbedTls& connectionAllocator, AllocatorConnectionMbedTlsListener& listenerAllocator, infra::BoundedList<ConnectionMbedTlsConnector>& connectors,
            ConnectionFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, ConnectionMbedTls::CertificateValidation certificateValidation = ConnectionMbedTls::CertificateValidation::Default);
        ~ConnectionFactoryMbedTls();

        infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& connectionObserverFactory, IPVersions versions = IPVersions::both) override;
        void Connect(ClientConnectionObserverFactory& connectionObserverFactory) override;
        void CancelConnect(ClientConnectionObserverFactory& connectionObserverFactory) override;

        infra::SharedPtr<ConnectionMbedTls> Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address);
        void Remove(ConnectionMbedTlsConnector& connector);

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
        bool clientSessionObtained = false;
        IPAddress previousAddress;
        ConnectionMbedTls::CertificateValidation certificateValidation;
    };

    class ConnectionFactoryWithNameResolverMbedTls
        : public ConnectionFactoryWithNameResolver
    {
    public:
        template<std::size_t MaxConnections, std::size_t MaxConnectors>
        using WithMaxConnectionsListenersAndConnectors =

            infra::WithStorage<
                infra::WithStorage<
                    ConnectionFactoryWithNameResolverMbedTls,
                    AllocatorConnectionMbedTls::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>,
                infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>::WithMaxSize<MaxConnectors>>;

        ConnectionFactoryWithNameResolverMbedTls(AllocatorConnectionMbedTls& connectionAllocator, infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>& connectors,
            ConnectionFactoryWithNameResolver& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, ConnectionMbedTls::CertificateValidation certificateValidation = ConnectionMbedTls::CertificateValidation::Default);
        ~ConnectionFactoryWithNameResolverMbedTls();

        void Connect(ClientConnectionObserverFactoryWithNameResolver& factory) override;
        void CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory) override;

        infra::SharedPtr<ConnectionMbedTls> Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, infra::BoundedConstString hostName);
        void Remove(ConnectionMbedTlsConnectorWithNameResolver& connector);

    private:
        void TryConnect();

    private:
        AllocatorConnectionMbedTls& connectionAllocator;
        infra::IntrusiveList<ClientConnectionObserverFactoryWithNameResolver> waitingConnects;
        infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>& connectors;
        ConnectionFactoryWithNameResolver& factory;
        CertificatesMbedTls& certificates;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        mbedtls_ssl_cache_context serverCache;
        mbedtls_ssl_session clientSession = {};
        bool clientSessionObtained = false;
        infra::BoundedConstString previousHostname;
        ConnectionMbedTls::CertificateValidation certificateValidation;
    };

    class ConnectionFactoryWithNameResolverForTls
        : public ConnectionFactoryWithNameResolver
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        explicit ConnectionFactoryWithNameResolverForTls(ConnectionFactoryWithNameResolver& connectionFactoryWithNameResolver);

        // Implementation of ConnectionFactoryWithNameResolver
        void Connect(ClientConnectionObserverFactoryWithNameResolver& factory) override;
        void CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory) override;

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    private:
        void TryConnect();

    private:
        ConnectionFactoryWithNameResolver& connectionFactoryWithNameResolver;
        ClientConnectionObserverFactoryWithNameResolver* clientConnectionFactory = nullptr;
        infra::IntrusiveList<ClientConnectionObserverFactoryWithNameResolver> waitingConnects;
    };
}

#endif
