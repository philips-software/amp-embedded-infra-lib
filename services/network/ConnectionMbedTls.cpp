#include "services/network/ConnectionMbedTls.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "services/network/CertificatesMbedTls.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    ConnectionMbedTls::ConnectionMbedTls(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, CertificatesMbedTls& certificates,
        hal::SynchronousRandomDataGenerator& randomDataGenerator, const ParametersWorkaround& parameters)
        : createdObserver(std::move(createdObserver))
        , randomDataGenerator(randomDataGenerator)
        , server(parameters.parameters.Is<ServerParameters>())
        , clientSession(parameters.parameters.Is<ClientParameters>() ? &parameters.parameters.Get<ClientParameters>().clientSession : nullptr)
        , clientSessionObtained(parameters.parameters.Is<ClientParameters>() ? &parameters.parameters.Get<ClientParameters>().clientSessionObtained : nullptr)
        , receiveReader([this]()
              {
                  keepAliveForReader = nullptr;
              })
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
        mbedtls_ssl_conf_authmode(&sslConfig, GetAuthMode(parameters));

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
        {
            TlsInitFailure(result);
            Abort();
        }
        else
        {
            if (!server && *clientSessionObtained)
            {
                result = mbedtls_ssl_set_session(&sslContext, clientSession);
                assert(result == 0);
            }

            mbedtls_ssl_set_bio(&sslContext, this, &ConnectionMbedTls::StaticSslSend, &ConnectionMbedTls::StaticSslReceive, nullptr);

            TryAllocateEncryptedSendStream();
        }
    }

    int ConnectionMbedTls::GetAuthMode(const ParametersWorkaround& parameters) const
    {
        auto certificateValidation = server ? parameters.parameters.Get<ServerParameters>().certificateValidation : parameters.parameters.Get<ClientParameters>().certificateValidation;

        switch (certificateValidation)
        {
            case CertificateValidation::Default:
                return server ? MBEDTLS_SSL_VERIFY_NONE : MBEDTLS_SSL_VERIFY_REQUIRED;
            case CertificateValidation::Disabled:
                return MBEDTLS_SSL_VERIFY_NONE;
            case CertificateValidation::Enabled:
                return MBEDTLS_SSL_VERIFY_REQUIRED;
            default:
                std::abort();
        }
    }

    ConnectionMbedTls::~ConnectionMbedTls()
    {
        encryptedSendWriter = nullptr;

        if (!destructed)
        {
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_ssl_free(&sslContext);
            mbedtls_ssl_config_free(&sslConfig);
        }
    }

    void ConnectionMbedTls::CreatedObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        if (connectionObserver != nullptr)
        {
            createdObserver(SharedFromThis());
            Attach(connectionObserver);
            if (aborting)
                Observer().Abort();
            else if (closing)
                Observer().Close();
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

            int result = mbedtls_ssl_read(&sslContext, buffer.begin(), buffer.size());
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
            else if (result == MBEDTLS_ERR_SSL_BAD_INPUT_DATA) // Precondition failure
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

        if (receiveBuffer.size() != startSize && !dataReceivedScheduled)
        {
            dataReceivedScheduled = true;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([this](const infra::SharedPtr<ConnectionMbedTls>& object)
                {
                    object->dataReceivedScheduled = false;
                    if (!receiveBuffer.empty() && object->Connection::IsAttached())
                        object->Observer().DataReceived();
                },
                SharedFromThis());
        }
    }

    void ConnectionMbedTls::Attached()
    {
        InitTls();
    }

    void ConnectionMbedTls::Detaching()
    {
        encryptedSendWriter = nullptr;

        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_ssl_free(&sslContext);
        mbedtls_ssl_config_free(&sslConfig);
        destructed = true;

        ConnectionWithHostname::Detach();
    }

    void ConnectionMbedTls::Close()
    {
        if (Connection::IsAttached())
            Observer().Close();
        else
            closing = true;
    }

    void ConnectionMbedTls::Abort()
    {
        if (Connection::IsAttached())
            Observer().Abort();
        else
            aborting = true;
    }

    void ConnectionMbedTls::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        really_assert(sendSize != 0 && sendSize <= MaxSendStreamSize());
        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t ConnectionMbedTls::MaxSendStreamSize() const
    {
        return sendBuffer.max_size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionMbedTls::ReceiveStream()
    {
        keepAliveForReader = SharedFromThis();
        return receiveReader.Emplace(*this);
    }

    void ConnectionMbedTls::AckReceived()
    {
        receiveReader->ConsumeRead();
        DataReceived();
    }

    void ConnectionMbedTls::CloseAndDestroy()
    {
        encryptedSendWriter = nullptr;
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void ConnectionMbedTls::AbortAndDestroy()
    {
        encryptedSendWriter = nullptr;
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    void ConnectionMbedTls::SetHostname(infra::BoundedConstString hostname)
    {
        terminatedHostname = hostname;
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
        if (!sending && requestedSendSize != 0)
        {
            assert(streamWriter.Allocatable());
            auto requestedSize = requestedSendSize;

            infra::EventDispatcherWithWeakPtr::Instance().Schedule([requestedSize](const infra::SharedPtr<ConnectionMbedTls>& object)
                {
                    infra::SharedPtr<StreamWriterMbedTls> stream = object->streamWriter.Emplace(*object, requestedSize);
                    if (object->Connection::IsAttached())
                        object->Observer().SendStreamAvailable(std::move(stream));
                },
                SharedFromThis());

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
        return reinterpret_cast<ConnectionMbedTls*>(context)->SslSend(infra::ConstByteRange(buffer, buffer + size));
    }

    int ConnectionMbedTls::StaticSslReceive(void* context, unsigned char* buffer, size_t size)
    {
        return reinterpret_cast<ConnectionMbedTls*>(context)->SslReceive(infra::ByteRange(buffer, buffer + size));
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
                    },
                    SharedFromThis());
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
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionMbedTls>& object)
                {
                    object->TrySend();
                },
                SharedFromThis());
            return streamBuffer.size();
        }
        else
            return MBEDTLS_ERR_SSL_WANT_READ;
    }

    void ConnectionMbedTls::TrySend()
    {
        while (!destructed && (initialHandshake || sending))
        {
            infra::ConstByteRange range = infra::MakeRange(sendBuffer);
            int result = initialHandshake
                             ? mbedtls_ssl_handshake(&sslContext)
                             : mbedtls_ssl_write(&sslContext, range.begin(), range.size());
            if (result == MBEDTLS_ERR_SSL_WANT_WRITE || result == MBEDTLS_ERR_SSL_WANT_READ)
                return;
            else if (result == MBEDTLS_ERR_SSL_BAD_INPUT_DATA) // Precondition failure
            {
                TlsWriteFailure(result);
                std::abort();
            }
            else if (result < 0)
            {
                encryptedSendWriter = nullptr;
                TlsWriteFailure(result);
                // Since TrySend may be invoked from the destructor of StreamWriterMbedTls, and since ConnectionMbedTls holds the storage of that stream writer,
                // we may not directly abort the connection here, since that would result in the stream writer not being able to be deallocted.
                // Instead, schedule the abort.
                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionMbedTls>& object)
                    {
                        object->ConnectionObserver::Subject().AbortAndDestroy();
                    },
                    SharedFromThis());
                return;
            }
            else if (initialHandshake)
            {
                initialHandshake = false;

                if (!server)
                {
                    if (!*clientSessionObtained)
                        mbedtls_ssl_session_init(clientSession);

                    result = mbedtls_ssl_get_session(&sslContext, clientSession);
                    *clientSessionObtained = true;
                    assert(result == 0);
                }
            }
            else
            {
                sendBuffer.erase(sendBuffer.begin(), sendBuffer.begin() + result);
                if (sendBuffer.empty())
                    sending = false;
                if (static_cast<std::size_t>(result) < range.size())
                    break;
            }
        }

        TryAllocateSendStream();
    }

    int ConnectionMbedTls::StaticGenerateRandomData(void* data, unsigned char* output, std::size_t size)
    {
        reinterpret_cast<ConnectionMbedTls*>(data)->GenerateRandomData(infra::ByteRange(output, output + size));
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

    ConnectionMbedTls::StreamWriterMbedTls::StreamWriterMbedTls(ConnectionMbedTls& connection, uint32_t size)
        : infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>(infra::inPlace, connection.sendBuffer, size)
        , connection(connection)
    {}

    ConnectionMbedTls::StreamWriterMbedTls::~StreamWriterMbedTls()
    {
        connection.sending = true;
        connection.TrySend();
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

    ConnectionMbedTlsListener::ConnectionMbedTlsListener(AllocatorConnectionMbedTls& allocator, ServerConnectionObserverFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, mbedtls_ssl_cache_context& serverCache, ConnectionMbedTls::CertificateValidation certificateValidation)
        : allocator(allocator)
        , factory(factory)
        , certificates(certificates)
        , randomDataGenerator(randomDataGenerator)
        , serverCache(serverCache)
        , certificateValidation(certificateValidation)
    {}

    void ConnectionMbedTlsListener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address)
    {
        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> creationFailed = createdObserver.Clone();
        infra::SharedPtr<ConnectionMbedTls> connection = allocator.Allocate(std::move(createdObserver), certificates, randomDataGenerator, { ConnectionMbedTls::ServerParameters{ serverCache, certificateValidation } });
        if (connection)
        {
            factory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
                {
                    connection->CreatedObserver(connectionObserver);
                },
                address);
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
        , networkFactory(networkFactory)
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
        infra::SharedPtr<ConnectionMbedTls> connection = factory.Allocate(std::move(createdObserver), clientFactory.Address());
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

    void ConnectionMbedTlsConnector::CancelConnect()
    {
        networkFactory.CancelConnect(*this);
    }

    ConnectionMbedTlsConnectorWithNameResolver::ConnectionMbedTlsConnectorWithNameResolver(ConnectionFactoryWithNameResolverMbedTls& factory, ConnectionFactoryWithNameResolver& networkFactory, ClientConnectionObserverFactoryWithNameResolver& clientFactory)
        : factory(factory)
        , networkFactory(networkFactory)
        , clientFactory(clientFactory)
    {
        networkFactory.Connect(*this);
    }

    infra::BoundedConstString ConnectionMbedTlsConnectorWithNameResolver::Hostname() const
    {
        return clientFactory.Hostname();
    }

    uint16_t ConnectionMbedTlsConnectorWithNameResolver::Port() const
    {
        return clientFactory.Port();
    }

    void ConnectionMbedTlsConnectorWithNameResolver::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> creationFailed = createdObserver.Clone();
        infra::SharedPtr<ConnectionMbedTls> connection = factory.Allocate(std::move(createdObserver), clientFactory.Hostname());
        if (connection)
        {
            connection->SetHostname(clientFactory.Hostname());
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

    void ConnectionMbedTlsConnectorWithNameResolver::ConnectionFailed(ConnectFailReason reason)
    {
        clientFactory.ConnectionFailed(reason);
        factory.Remove(*this);
    }

    void ConnectionMbedTlsConnectorWithNameResolver::CancelConnect()
    {
        networkFactory.CancelConnect(*this);
    }

    ConnectionFactoryMbedTls::ConnectionFactoryMbedTls(AllocatorConnectionMbedTls& connectionAllocator,
        AllocatorConnectionMbedTlsListener& listenerAllocator, infra::BoundedList<ConnectionMbedTlsConnector>& connectors,
        ConnectionFactory& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, ConnectionMbedTls::CertificateValidation certificateValidation)
        : connectionAllocator(connectionAllocator)
        , listenerAllocator(listenerAllocator)
        , connectors(connectors)
        , factory(factory)
        , certificates(certificates)
        , randomDataGenerator(randomDataGenerator)
        , certificateValidation(certificateValidation)
    {
        mbedtls_ssl_cache_init(&serverCache);
        mbedtls_ssl_session_init(&clientSession);
    }

    ConnectionFactoryMbedTls::~ConnectionFactoryMbedTls()
    {
        mbedtls_ssl_cache_free(&serverCache);
        if (clientSessionObtained)
        {
            mbedtls_ssl_session_free(&clientSession);
            clientSessionObtained = false;
        }
    }

    infra::SharedPtr<void> ConnectionFactoryMbedTls::Listen(uint16_t port, ServerConnectionObserverFactory& connectionObserverFactory, IPVersions versions)
    {
        infra::SharedPtr<ConnectionMbedTlsListener> listener = listenerAllocator.Allocate(connectionAllocator, connectionObserverFactory, certificates,
            randomDataGenerator, serverCache, certificateValidation);

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
                    connector.CancelConnect();
                    connectors.remove(connector);
                    return;
                }

            std::abort();
        }
    }

    infra::SharedPtr<ConnectionMbedTls> ConnectionFactoryMbedTls::Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        if (address != previousAddress)
        {
            if (clientSessionObtained)
            {
                mbedtls_ssl_session_free(&clientSession);
                clientSessionObtained = false;
            }
            clientSession = mbedtls_ssl_session();
        }

        previousAddress = address;

        return connectionAllocator.Allocate(std::move(createdObserver), certificates, randomDataGenerator, { ConnectionMbedTls::ClientParameters{ clientSession, clientSessionObtained, certificateValidation } });
    }

    void ConnectionFactoryMbedTls::Remove(ConnectionMbedTlsConnector& connector)
    {
        connectors.remove(connector);

        TryConnect();
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

    ConnectionFactoryWithNameResolverMbedTls::ConnectionFactoryWithNameResolverMbedTls(AllocatorConnectionMbedTls& connectionAllocator, infra::BoundedList<ConnectionMbedTlsConnectorWithNameResolver>& connectors,
        ConnectionFactoryWithNameResolver& factory, CertificatesMbedTls& certificates, hal::SynchronousRandomDataGenerator& randomDataGenerator, ConnectionMbedTls::CertificateValidation certificateValidation)
        : connectionAllocator(connectionAllocator)
        , connectors(connectors)
        , factory(factory)
        , certificates(certificates)
        , randomDataGenerator(randomDataGenerator)
        , certificateValidation(certificateValidation)
    {
        mbedtls_ssl_cache_init(&serverCache);
        mbedtls_ssl_session_init(&clientSession);
    }

    ConnectionFactoryWithNameResolverMbedTls::~ConnectionFactoryWithNameResolverMbedTls()
    {
        mbedtls_ssl_cache_free(&serverCache);
        if (clientSessionObtained)
        {
            mbedtls_ssl_session_free(&clientSession);
            clientSessionObtained = false;
        }
    }

    void ConnectionFactoryWithNameResolverMbedTls::Connect(ClientConnectionObserverFactoryWithNameResolver& connectionObserverFactory)
    {
        waitingConnects.push_back(connectionObserverFactory);

        TryConnect();
    }

    void ConnectionFactoryWithNameResolverMbedTls::CancelConnect(ClientConnectionObserverFactoryWithNameResolver& connectionObserverFactory)
    {
        if (waitingConnects.has_element(connectionObserverFactory))
            waitingConnects.erase(connectionObserverFactory);
        else
        {
            for (auto& connector : connectors)
                if (&connector.clientFactory == &connectionObserverFactory)
                {
                    connector.CancelConnect();
                    connectors.remove(connector);
                    return;
                }

            std::abort();
        }
    }

    infra::SharedPtr<ConnectionMbedTls> ConnectionFactoryWithNameResolverMbedTls::Allocate(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, infra::BoundedConstString hostname)
    {
        if (hostname != previousHostname)
        {
            if (clientSessionObtained)
            {
                mbedtls_ssl_session_free(&clientSession);
                clientSessionObtained = false;
            }
            clientSession = mbedtls_ssl_session();
        }

        previousHostname = hostname;

        return connectionAllocator.Allocate(std::move(createdObserver), certificates, randomDataGenerator, { ConnectionMbedTls::ClientParameters{ clientSession, clientSessionObtained, certificateValidation } });
    }

    void ConnectionFactoryWithNameResolverMbedTls::Remove(ConnectionMbedTlsConnectorWithNameResolver& connector)
    {
        connectors.remove(connector);

        TryConnect();
    }

    void ConnectionFactoryWithNameResolverMbedTls::TryConnect()
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
        waitingConnects.push_back(factory);
        TryConnect();
    }

    void ConnectionFactoryWithNameResolverForTls::CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        if (clientConnectionFactory == &factory)
        {
            connectionFactoryWithNameResolver.CancelConnect(*this);
            clientConnectionFactory = nullptr;
            TryConnect();
        }
        else
            waitingConnects.erase(factory);
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
        hostname = Hostname(); // After ConnectionEstablished Hostname may not be invoked
        clientConnectionFactory->ConnectionEstablished([this, &createdObserver](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                createdObserver(connectionObserver);
                if (connectionObserver->IsAttached())
                    static_cast<ConnectionWithHostname&>(connectionObserver->Subject()).SetHostname(hostname);
            });
        clientConnectionFactory = nullptr;
        TryConnect();
    }

    void ConnectionFactoryWithNameResolverForTls::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientConnectionFactory != nullptr);
        clientConnectionFactory->ConnectionFailed(reason);
        clientConnectionFactory = nullptr;
        TryConnect();
    }

    void ConnectionFactoryWithNameResolverForTls::TryConnect()
    {
        if (clientConnectionFactory == nullptr && !waitingConnects.empty())
        {
            clientConnectionFactory = &waitingConnects.front();
            waitingConnects.pop_front();
            connectionFactoryWithNameResolver.Connect(*this);
        }
    }
}
