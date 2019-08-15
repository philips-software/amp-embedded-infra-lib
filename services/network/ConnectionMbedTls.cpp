#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "services/network/CertificatesMbedTls.hpp"
#include "services/network/ConnectionMbedTls.hpp"

namespace services
{
    ConnectionMbedTls::ConnectionMbedTls(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates,
        hal::SynchronousRandomDataGenerator& randomDataGenerator, const ParametersWorkaround& parameters)
        : createdObserver(std::move(createdObserver))
        , randomDataGenerator(randomDataGenerator)
        , server(parameters.parameters.Is<ServerParameters>())
        , clientSession(parameters.parameters.Is<ClientParameters>() ? &parameters.parameters.Get<ClientParameters>().clientSession : nullptr)
    {
        mbedtls_ssl_init(&sslContext);
        mbedtls_ssl_config_init(&sslConfig);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_ssl_conf_dbg(&sslConfig, StaticDebugWrapper, this);

        int result;

        result = mbedtls_ctr_drbg_seed(&ctr_drbg, &ConnectionMbedTls::StaticGenerateRandomData, this, nullptr, 0);
        assert(result == 0);

        result = mbedtls_ssl_config_defaults(&sslConfig, server ? MBEDTLS_SSL_IS_SERVER : MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
        assert(result == 0);
        mbedtls_ssl_conf_rng(&sslConfig, mbedtls_ctr_drbg_random, &ctr_drbg);
        mbedtls_ssl_conf_authmode(&sslConfig, (server && !parameters.parameters.Get<ServerParameters>().clientAuthenticationNeeded) ? MBEDTLS_SSL_VERIFY_NONE : MBEDTLS_SSL_VERIFY_REQUIRED);

        certificates.Config(sslConfig);

        if (server)
            mbedtls_ssl_conf_session_cache(&sslConfig, &parameters.parameters.Get<ServerParameters>().serverCache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);

        if (!server)
        {
            // mbedTLS can only talk with itself if the client enables ALPN. ALPN requires at least one protocol.
            static const char* protos[] = { "http/1.1", nullptr };
            mbedtls_ssl_conf_alpn_protocols(&sslConfig, protos);
        }
    }

    void ConnectionMbedTls::InitTls()
    {
        int result = mbedtls_ssl_setup(&sslContext, &sslConfig);
        if (result != 0)
            TlsInitFailure(result);
        really_assert(result == 0);

        if (!server)
        {
            if (clientSession->id_len != 0) // Workaround for what seems to be a problem in mbedTLS: When id_len = 0, then mbedTLS still thinks it can resume the session presented by the server
            {
                result = mbedtls_ssl_set_session(&sslContext, clientSession);
                assert(result == 0);
            }
        }

        mbedtls_ssl_set_bio(&sslContext, this, &ConnectionMbedTls::StaticSslSend, &ConnectionMbedTls::StaticSslReceive, nullptr);

        TryAllocateEncryptedSendStream();
    }

    ConnectionMbedTls::~ConnectionMbedTls()
    {
        encryptedSendWriter = nullptr;

        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_ssl_free(&sslContext);
        mbedtls_ssl_config_free(&sslConfig);

        ResetOwnership();
    }

    void ConnectionMbedTls::CreatedObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        if (connectionObserver != nullptr)
        {
            connectionObserver->Attach(*this);
            SetOwnership(nullptr, connectionObserver);
            createdObserver(SharedFromThis());
        }
        else
            createdObserver(nullptr);
    }

    void ConnectionMbedTls::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        encryptedSendWriter = writer;
        TrySend();
    }

    void ConnectionMbedTls::DataReceived()
    {
        std::size_t startSize = receiveBuffer.size();

        while (!receiveBuffer.full())
        {
            std::size_t newBufferStart = receiveBuffer.size();
            receiveBuffer.resize(receiveBuffer.max_size());
            infra::ByteRange buffer = receiveBuffer.contiguous_range(receiveBuffer.begin() + newBufferStart);

            int result = mbedtls_ssl_read(&sslContext, reinterpret_cast<unsigned char*>(buffer.begin()), buffer.size());
            if (result == MBEDTLS_ERR_SSL_WANT_WRITE || result == MBEDTLS_ERR_SSL_WANT_READ)
            {
                receiveBuffer.resize(newBufferStart);
                break;
            }
            else if (result == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            {
                receiveBuffer.resize(newBufferStart);
                break;
            }
            else if (result == MBEDTLS_ERR_SSL_BAD_INPUT_DATA)  // Precondition failure
            {
                TlsReadFailure(result);
                std::abort();
            }
            else if (result < 0)
            {
                encryptedSendWriter = nullptr;
                TlsReadFailure(result);
                ConnectionObserver::Subject().AbortAndDestroy();
                return;
            }
            else
                receiveBuffer.resize(newBufferStart + static_cast<std::size_t>(result));
        }

        if (receiveBuffer.size() != startSize)
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionMbedTls>& object)
            {
                object->GetObserver().DataReceived();
            }, SharedFromThis());
    }

    void ConnectionMbedTls::Connected()
    {
        InitTls();
        GetObserver().Connected();
    }

    void ConnectionMbedTls::ClosingConnection()
    {
        encryptedSendWriter = nullptr;
        ResetOwnership();
    }

    void ConnectionMbedTls::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        assert(sendSize != 0 && sendSize <= MaxSendStreamSize());
        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t ConnectionMbedTls::MaxSendStreamSize() const
    {
        return sendBuffer.max_size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionMbedTls::ReceiveStream()
    {
        return receiveReader.Emplace(*this);
    }

    void ConnectionMbedTls::AckReceived()
    {
        receiveReader->ConsumeRead();
        DataReceived();
    }

    void ConnectionMbedTls::CloseAndDestroy()
    {
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void ConnectionMbedTls::AbortAndDestroy()
    {
        encryptedSendWriter = nullptr;
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    void ConnectionMbedTls::SetHostname(infra::BoundedConstString hostname)
    {
        infra::BoundedString::WithStorage<MBEDTLS_SSL_MAX_HOST_NAME_LEN + 1> terminatedHostname(hostname);
        terminatedHostname.push_back(0);

        int result = mbedtls_ssl_set_hostname(&sslContext, terminatedHostname.data());
        assert(result == 0);
    }

    void ConnectionMbedTls::TlsInitFailure(int reason)
    {}

    void ConnectionMbedTls::TlsReadFailure(int reason)
    {}

    void ConnectionMbedTls::TlsWriteFailure(int reason)
    {}

    void ConnectionMbedTls::TlsLog(int level, const char* file, int line, const char* message)
    {}

    void ConnectionMbedTls::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (sendBuffer.max_size() - sendBuffer.size() >= requestedSendSize)
        {
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionMbedTls>& object)
            {
                infra::SharedPtr<StreamWriterMbedTls> stream = object->streamWriter.Emplace(*object);
                object->GetObserver().SendStreamAvailable(std::move(stream));
            }, SharedFromThis());

            requestedSendSize = 0;
        }
    }

    void ConnectionMbedTls::TryAllocateEncryptedSendStream()
    {
        encryptedSendStreamSize = ConnectionObserver::Subject().MaxSendStreamSize();
        ConnectionObserver::Subject().RequestSendStream(encryptedSendStreamSize);
    }

    int ConnectionMbedTls::StaticSslSend(void* context, const unsigned char* buffer, std::size_t size)
    {
        return reinterpret_cast<ConnectionMbedTls*>(context)->SslSend(infra::ConstByteRange(reinterpret_cast<const uint8_t*>(buffer), reinterpret_cast<const uint8_t*>(buffer) + size));
    }

    int ConnectionMbedTls::StaticSslReceive(void *context, unsigned char* buffer, size_t size)
    {
        return reinterpret_cast<ConnectionMbedTls*>(context)->SslReceive(infra::ByteRange(reinterpret_cast<uint8_t*>(buffer), reinterpret_cast<uint8_t*>(buffer) + size));
    }

    int ConnectionMbedTls::SslSend(infra::ConstByteRange buffer)
    {
        if (encryptedSendWriter && encryptedSendStreamSize != 0)
        {
            buffer = infra::Head(buffer, encryptedSendStreamSize);
            encryptedSendStreamSize -= buffer.size();
            infra::DataOutputStream::WithErrorPolicy stream(*encryptedSendWriter);
            stream << buffer;

            if (!buffer.empty() && !flushScheduled)
            {
                flushScheduled = true;
                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionMbedTls>& object)
                {
                    object->flushScheduled = false;
                    object->encryptedSendWriter = nullptr;
                    object->TryAllocateEncryptedSendStream();
                }, SharedFromThis());
            }

            return buffer.size();
        }
        else
            return MBEDTLS_ERR_SSL_WANT_WRITE;
    }

    int ConnectionMbedTls::SslReceive(infra::ByteRange buffer)
    {
        infra::SharedPtr<infra::StreamReader> reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);
        infra::ConstByteRange streamBuffer = stream.ContiguousRange(buffer.size());
        std::copy(streamBuffer.begin(), streamBuffer.end(), buffer.begin());
        ConnectionObserver::Subject().AckReceived();

        if (!streamBuffer.empty())
        {
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionMbedTls>& object) { object->TrySend(); }, SharedFromThis());
            return streamBuffer.size();
        }
        else
            return MBEDTLS_ERR_SSL_WANT_READ;
    }

    void ConnectionMbedTls::TrySend()
    {
        while (initialHandshake || !sendBuffer.empty())
        {
            infra::ConstByteRange range = sendBuffer.contiguous_range(sendBuffer.begin());
            int result = initialHandshake
                ? mbedtls_ssl_handshake(&sslContext)
                : mbedtls_ssl_write(&sslContext, reinterpret_cast<const unsigned char*>(range.begin()), range.size());
            if (result == MBEDTLS_ERR_SSL_WANT_WRITE || result == MBEDTLS_ERR_SSL_WANT_READ)
                return;
            else if (result == MBEDTLS_ERR_SSL_BAD_INPUT_DATA)  // Precondition failure
            {
                TlsWriteFailure(result);
                std::abort();
            }
            else if (result < 0)
            {
                encryptedSendWriter = nullptr;
                TlsWriteFailure(result);
                ConnectionObserver::Subject().AbortAndDestroy();
                return;
            }
            else if (initialHandshake)
            {
                initialHandshake = false;

                if (!server)
                {
                    result = mbedtls_ssl_get_session(&sslContext, clientSession);
                    assert(result == 0);
                }
            }
            else
            {
                sendBuffer.erase(sendBuffer.begin(), sendBuffer.begin() + result);
                if (static_cast<std::size_t>(result) < range.size())
                    break;
            }
        }

        if (requestedSendSize != 0)
            TryAllocateSendStream();
    }

    int ConnectionMbedTls::StaticGenerateRandomData(void* data, unsigned char* output, std::size_t size)
    {
        reinterpret_cast<ConnectionMbedTls*>(data)->GenerateRandomData(infra::ByteRange(reinterpret_cast<uint8_t*>(output), reinterpret_cast<uint8_t*>(output) + size));
        return 0;
    }

    void ConnectionMbedTls::GenerateRandomData(infra::ByteRange data)
    {
        randomDataGenerator.GenerateRandomData(data);
    }

    void ConnectionMbedTls::StaticDebugWrapper(void* context, int level, const char* file, int line, const char* message)
    {
        reinterpret_cast<ConnectionMbedTls*>(context)->TlsLog(level, file, line, message);
    }

    ConnectionMbedTls::StreamWriterMbedTls::StreamWriterMbedTls(ConnectionMbedTls& connection)
        : connection(connection)
    {}

    ConnectionMbedTls::StreamWriterMbedTls::~StreamWriterMbedTls()
    {
        if (sent != 0)
            connection.TrySend();
    }

    void ConnectionMbedTls::StreamWriterMbedTls::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        connection.sendBuffer.insert(connection.sendBuffer.end(), range.begin(), range.end());
        sent += range.size();
    }

    std::size_t ConnectionMbedTls::StreamWriterMbedTls::Available() const
    {
        return connection.sendBuffer.max_size() - sent;
    }

    ConnectionMbedTls::StreamReaderMbedTls::StreamReaderMbedTls(ConnectionMbedTls& connection)
        : infra::BoundedDequeInputStreamReader(connection.receiveBuffer)
        , connection(connection)
    {}

    void ConnectionMbedTls::StreamReaderMbedTls::ConsumeRead()
    {
        connection.receiveBuffer.erase(connection.receiveBuffer.begin(), connection.receiveBuffer.begin() + ConstructSaveMarker());
        Rewind(0);
    }

    ConnectionMbedTlsListener::ConnectionMbedTlsListener(AllocatorConnectionMbedTls& allocator, ServerConnectionObserverFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, mbedtls_ssl_cache_context& serverCache, bool clientAuthenticationNeeded)
        : allocator(allocator)
        , factory(factory)
        , certificates(certificates)
        , randomDataGenerator(randomDataGenerator)
        , serverCache(serverCache)
        , clientAuthenticationNeeded(clientAuthenticationNeeded)
    {}

    void ConnectionMbedTlsListener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address)
    {
        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> creationFailed = createdObserver.Clone();
        infra::SharedPtr<ConnectionMbedTls> connection = allocator.Allocate(std::move(createdObserver), certificates, randomDataGenerator, { ConnectionMbedTls::ServerParameters{ serverCache, clientAuthenticationNeeded } });
        if (connection)
        {
            factory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                connection->CreatedObserver(connectionObserver);
            }, address);
        }
        else
            creationFailed(nullptr);
    }

    void ConnectionMbedTlsListener::SetListener(infra::SharedPtr<void> listener)
    {
        this->listener = listener;
    }

    ConnectionMbedTlsConnector::ConnectionMbedTlsConnector(ConnectionFactoryMbedTls& factory, ConnectionFactory& networkFactory, ClientConnectionObserverFactory& clientFactory)
        : factory(factory)
        , clientFactory(clientFactory)
    {
        networkFactory.Connect(*this);
    }

    IPAddress ConnectionMbedTlsConnector::Address() const
    {
        return clientFactory.Address();
    }

    uint16_t ConnectionMbedTlsConnector::Port() const
    {
        return clientFactory.Port();
    }

    void ConnectionMbedTlsConnector::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> creationFailed = createdObserver.Clone();
        infra::SharedPtr<ConnectionMbedTls> connection = factory.Allocate(std::move(createdObserver));
        if (connection)
        {
            clientFactory.ConnectionEstablished([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                connection->CreatedObserver(connectionObserver);
            });

            factory.Remove(*this);
        }
        else
        {
            ConnectionFailed(ConnectFailReason::connectionAllocationFailed);
            creationFailed(nullptr);
        }
    }

    void ConnectionMbedTlsConnector::ConnectionFailed(ConnectFailReason reason)
    {
        clientFactory.ConnectionFailed(reason);
        factory.Remove(*this);
    }

    ConnectionFactoryMbedTls::ConnectionFactoryMbedTls(AllocatorConnectionMbedTls& connectionAllocator,
        AllocatorConnectionMbedTlsListener& listenerAllocator, infra::BoundedList<ConnectionMbedTlsConnector>& connectors,
        ConnectionFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, bool needsAuthenticationDefault)
        : connectionAllocator(connectionAllocator)
        , listenerAllocator(listenerAllocator)
        , connectors(connectors)
        , factory(factory)
        , certificates(certificates)
        , randomDataGenerator(randomDataGenerator)
        , needsAuthenticationDefault(needsAuthenticationDefault)
    {
        mbedtls_ssl_cache_init(&serverCache);
        mbedtls_ssl_session_init(&clientSession);
    }

    ConnectionFactoryMbedTls::~ConnectionFactoryMbedTls()
    {
        mbedtls_ssl_cache_free(&serverCache);
        mbedtls_ssl_session_free(&clientSession);
    }

    infra::SharedPtr<void> ConnectionFactoryMbedTls::Listen(uint16_t port, ServerConnectionObserverFactory& connectionObserverFactory, IPVersions versions)
    {
        infra::SharedPtr<ConnectionMbedTlsListener> listener = listenerAllocator.Allocate(connectionAllocator, connectionObserverFactory, certificates, randomDataGenerator, serverCache, NeedsAuthentication(port));

        if (listener)
        {
            infra::SharedPtr<void> networkListener = factory.Listen(port, *listener, versions);
            if (networkListener)
            {
                listener->SetListener(networkListener);
                return listener;
            }
        }

        return nullptr;
    }

    void ConnectionFactoryMbedTls::Connect(ClientConnectionObserverFactory& connectionObserverFactory)
    {
        waitingConnects.push_back(connectionObserverFactory);

        TryConnect();
    }

    void ConnectionFactoryMbedTls::CancelConnect(ClientConnectionObserverFactory& connectionObserverFactory)
    {
        if (waitingConnects.has_element(connectionObserverFactory))
            waitingConnects.erase(connectionObserverFactory);
        else
        {
            for (auto& connector : connectors)
                if (&connector.clientFactory == &connectionObserverFactory)
                {
                    connectors.remove(connector);
                    return;
                }

            std::abort();
        }
    }

    infra::SharedPtr<ConnectionMbedTls> ConnectionFactoryMbedTls::Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        return connectionAllocator.Allocate(std::move(createdObserver), certificates, randomDataGenerator, { ConnectionMbedTls::ClientParameters{ clientSession } });
    }

    void ConnectionFactoryMbedTls::Remove(ConnectionMbedTlsConnector& connector)
    {
        connectors.remove(connector);

        TryConnect();
    }

    bool ConnectionFactoryMbedTls::NeedsAuthentication(uint16_t port) const
    {
        return needsAuthenticationDefault;
    }

    void ConnectionFactoryMbedTls::TryConnect()
    {
        if (!waitingConnects.empty() && !connectors.full())
        {
            auto& connectionObserverFactory = waitingConnects.front();
            waitingConnects.pop_front();

            connectors.emplace_back(*this, factory, connectionObserverFactory);
        }
    }

    ConnectionFactoryWithNameResolverForTls::ConnectionFactoryWithNameResolverForTls(ConnectionFactoryWithNameResolver& connectionFactoryWithNameResolver)
        : connectionFactoryWithNameResolver(connectionFactoryWithNameResolver)
    {}

    void ConnectionFactoryWithNameResolverForTls::Connect(ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        clientConnectionFactory = &factory;
        connectionFactoryWithNameResolver.Connect(*this);
    }

    void ConnectionFactoryWithNameResolverForTls::CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        clientConnectionFactory = &factory;
        connectionFactoryWithNameResolver.CancelConnect(*this);
    }

    infra::BoundedConstString ConnectionFactoryWithNameResolverForTls::Hostname() const
    {
        assert(clientConnectionFactory != nullptr);
        return clientConnectionFactory->Hostname();
    }

    uint16_t ConnectionFactoryWithNameResolverForTls::Port() const
    {
        assert(clientConnectionFactory != nullptr);
        return clientConnectionFactory->Port();
    }

    void ConnectionFactoryWithNameResolverForTls::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientConnectionFactory != nullptr);
        clientConnectionFactory->ConnectionEstablished([this, &createdObserver](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            createdObserver(connectionObserver);
            static_cast<ConnectionWithHostname&>(connectionObserver->Subject()).SetHostname(Hostname());
        });
    }

    void ConnectionFactoryWithNameResolverForTls::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientConnectionFactory != nullptr);
        return clientConnectionFactory->ConnectionFailed(reason);
    }
}
