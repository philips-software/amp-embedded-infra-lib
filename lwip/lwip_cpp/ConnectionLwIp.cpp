#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "lwip/lwip_cpp/ConnectionLwIp.hpp"

namespace services
{
    ConnectionLwIp::ConnectionLwIp(tcp_pcb* control)
        : control(control)
    {
        assert(control != nullptr);
        tcp_arg(control, this);
        tcp_recv(control, &ConnectionLwIp::Recv);
        tcp_err(control, &ConnectionLwIp::Err);
        tcp_sent(control, &ConnectionLwIp::Sent);
        tcp_nagle_disable(control);
    }

    ConnectionLwIp::~ConnectionLwIp()
    {
        if (control)
        {
            tcp_pcb* c = control;
            ResetControl();
            tcp_abort(c);
        }
    }

    void ConnectionLwIp::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        assert(sendSize != 0 && sendSize <= MaxSendStreamSize());
        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t ConnectionLwIp::MaxSendStreamSize() const
    {
        return tcp_mss(control);
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionLwIp::ReceiveStream()
    {
        return streamReader.Emplace(*this);
    }

    void ConnectionLwIp::AckReceived()
    {
        streamReader->ConsumeRead();
    }

    void ConnectionLwIp::CloseAndDestroy()
    {
        err_t result = tcp_close(control);
        if (result != ERR_OK)
            AbortAndDestroy();
        else
        {
            ResetControl();
            ResetOwnership();
        }
    }

    void ConnectionLwIp::AbortAndDestroy()
    {
        tcp_abort(control); // Err is called as a result, and this callback destroys this connection object
    }

    IPAddress ConnectionLwIp::IpAddress() const
    {
        if (control->remote_ip.type == IPADDR_TYPE_V4)
            return IPv4Address{
                ip4_addr1(ip_2_ip4(&control->remote_ip)),
                ip4_addr2(ip_2_ip4(&control->remote_ip)),
                ip4_addr3(ip_2_ip4(&control->remote_ip)),
                ip4_addr4(ip_2_ip4(&control->remote_ip))
        };
        else
            return IPv6Address{
                IP6_ADDR_BLOCK1(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK2(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK3(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK4(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK5(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK6(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK7(ip_2_ip6(&control->remote_ip)),
                IP6_ADDR_BLOCK8(ip_2_ip6(&control->remote_ip))
        };
    }

    void ConnectionLwIp::SendBuffer(infra::ConstByteRange buffer)
    {
        err_t result = tcp_write(control, buffer.begin(), static_cast<uint16_t>(buffer.size()), 0);
        if (result == ERR_OK)
        {
            tcp_output(control);
            sendBuffers.push_back(buffer);
            sendBuffer.clear();
        }
        else
        {
            sendBuffer = buffer;
            retrySendTimer.Start(std::chrono::milliseconds(50), [this]() { SendBuffer(sendBuffer); });
        }
    }

    void ConnectionLwIp::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffers.full() && !sendMemoryPool.full() && sendBuffer.empty())
        {
            sendMemoryPool.emplace_back();
            infra::ByteRange sendBuffer = infra::Head(infra::ByteRange(sendMemoryPool.back()), requestedSendSize);
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([sendBuffer](const infra::SharedPtr<ConnectionLwIp>& self)
            {
                infra::SharedPtr<infra::StreamWriter> stream = self->streamWriter.Emplace(*self, sendBuffer);
                self->GetObserver().SendStreamAvailable(std::move(stream));
            }, SharedFromThis());

            requestedSendSize = 0;
        }
    }

    void ConnectionLwIp::ResetControl()
    {
        tcp_arg(control, nullptr);
        tcp_recv(control, nullptr);
        tcp_err(control, nullptr);
        tcp_sent(control, nullptr);
        control = nullptr;
    }

    err_t ConnectionLwIp::Recv(void* arg, tcp_pcb* tpcb, pbuf* p, err_t err)
    {
        return static_cast<ConnectionLwIp*>(arg)->Recv(p, err);
    }

    void ConnectionLwIp::Err(void* arg, err_t err)
    {
        static_cast<ConnectionLwIp*>(arg)->Err(err);
    }

    err_t ConnectionLwIp::Sent(void* arg, struct tcp_pcb* tpcb, std::uint16_t len)
    {
        return static_cast<ConnectionLwIp*>(arg)->Sent(len);
    }

    err_t ConnectionLwIp::Recv(pbuf* p, err_t err)
    {
        if (p != nullptr)
        {
            for (pbuf* q = p; q != nullptr; q = q->next)
                receiveBuffer.insert(receiveBuffer.end(), reinterpret_cast<uint8_t*>(q->payload), reinterpret_cast<uint8_t*>(q->payload) + q->len);
            pbuf_free(p);

            if (!dataReceivedScheduled && HasObserver())
            {
                dataReceivedScheduled = true;
                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionLwIp>& self)
                {
                    self->dataReceivedScheduled = false;
                    self->GetObserver().DataReceived();
                }, SharedFromThis());
            }
        }
        else
            ResetOwnership();

        return ERR_OK;
    }

    void ConnectionLwIp::Err(err_t err)
    {
        assert(err == ERR_RST || err == ERR_CLSD || err == ERR_ABRT);
        ResetControl();
        ResetOwnership();
    }

    err_t ConnectionLwIp::Sent(std::uint16_t len)
    {
        while (len != 0)
        {
            assert(!sendBuffers.empty());
            if (sendBuffers.front().size() <= len)
            {
                len -= static_cast<uint16_t>(sendBuffers.front().size());
                sendBuffers.pop_front();
                sendMemoryPool.pop_front();
            }
            else
            {
                sendBuffers.front().pop_front(len);
                len = 0;
            }
        }

        if (requestedSendSize != 0)
            TryAllocateSendStream();

        return ERR_OK;
    }

    ConnectionLwIp::StreamWriterLwIp::StreamWriterLwIp(ConnectionLwIp& connection, infra::ByteRange sendBuffer)
        : infra::ByteOutputStreamWriter(sendBuffer)
        , connection(connection)
    {}

    ConnectionLwIp::StreamWriterLwIp::~StreamWriterLwIp()
    {
        if (!Processed().empty() && connection.control != nullptr)
            connection.SendBuffer(Processed());
        else
            connection.sendMemoryPool.pop_back();
    }

    ConnectionLwIp::StreamReaderLwIp::StreamReaderLwIp(ConnectionLwIp& connection)
        : infra::BoundedDequeInputStreamReader(connection.receiveBuffer)
        , connection(connection)
    {}

    void ConnectionLwIp::StreamReaderLwIp::ConsumeRead()
    {
        uint16_t sizeRead = static_cast<uint16_t>(ConstructSaveMarker());
        tcp_recved(connection.control, sizeRead);
        connection.receiveBuffer.erase(connection.receiveBuffer.begin(), connection.receiveBuffer.begin() + sizeRead);
        Rewind(0);
    }

    ListenerLwIp::ListenerLwIp(AllocatorConnectionLwIp& allocator, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
        : allocator(allocator)
        , factory(factory)
    {
        tcp_pcb* pcb = tcp_new();
        assert(pcb != nullptr);
        if (versions == IPVersions::both)
            err_t err = tcp_bind(pcb, IP_ADDR_ANY, port);
        else if (versions == IPVersions::ipv4)
#ifdef ESP_PLATFORM
            err_t err = tcp_bind(pcb, IP_ADDR_ANY, port);
#else
            err_t err = tcp_bind(pcb, IP4_ADDR_ANY, port);
#endif
        else
            err_t err = tcp_bind(pcb, IP6_ADDR_ANY, port);
        listenPort = tcp_listen(pcb);
        assert(listenPort != nullptr);
        tcp_accept(listenPort, &ListenerLwIp::Accept);
        tcp_arg(listenPort, this);
    }

    ListenerLwIp::~ListenerLwIp()
    {
        tcp_close(listenPort);
    }

    err_t ListenerLwIp::Accept(void* arg, struct tcp_pcb* newPcb, err_t err)
    {
        return static_cast<ListenerLwIp*>(arg)->Accept(newPcb, err);
    }

    err_t ListenerLwIp::Accept(tcp_pcb* newPcb, err_t err)
    {
        tcp_accepted(listenPort);
        infra::SharedPtr<ConnectionLwIp> connection = allocator.Allocate(newPcb);
        if (connection)
        {
            factory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver)
                {
                    connectionObserver->Attach(*connection);
                    connection->SetOwnership(connection, connectionObserver);
                    connectionObserver->Connected();
                }
            }, connection->IpAddress());

            infra::WeakPtr<ConnectionLwIp> weakConnection = connection;
            connection = nullptr;
            if (weakConnection.lock())
                return ERR_OK;
            else
                return ERR_ABRT;
        }
        else
        {
            tcp_abort(newPcb);
            return ERR_ABRT;
        }
    }

    ConnectorLwIp::ConnectorLwIp(ConnectionFactoryLwIp& factory, ClientConnectionObserverFactory& clientFactory, AllocatorConnectionLwIp& connectionAllocator)
        : factory(factory)
        , clientFactory(clientFactory)
        , connectionAllocator(connectionAllocator)
        , control(tcp_new())
    {
        tcp_arg(control, this);
        tcp_err(control, &ConnectorLwIp::StaticError);

        if (clientFactory.Address().Is<IPv4Address>())
        {
            IPv4Address ipv4Address = clientFactory.Address().Get<IPv4Address>();
            ip_addr_t ipAddress IPADDR4_INIT(0);
            IP4_ADDR(&ipAddress.u_addr.ip4, ipv4Address[0], ipv4Address[1], ipv4Address[2], ipv4Address[3]);
            err_t result = tcp_connect(control, &ipAddress, clientFactory.Port(), &ConnectorLwIp::StaticConnected);
            assert(result == ERR_OK);
        }
        else
        {
            IPv6Address ipv6Address = clientFactory.Address().Get<IPv6Address>();
            ip_addr_t ipAddress IPADDR6_INIT(0, 0, 0, 0);
            IP6_ADDR(&ipAddress.u_addr.ip6, PP_HTONL(ipv6Address[1] + (static_cast<uint32_t>(ipv6Address[0]) << 16)), PP_HTONL(ipv6Address[3] + (static_cast<uint32_t>(ipv6Address[2]) << 16)), PP_HTONL(ipv6Address[5] + (static_cast<uint32_t>(ipv6Address[4]) << 16)), PP_HTONL(ipv6Address[7] + (static_cast<uint32_t>(ipv6Address[6]) << 16)));
            err_t result = tcp_connect(control, &ipAddress, clientFactory.Port(), &ConnectorLwIp::StaticConnected);
            assert(result == ERR_OK);
        }
    }

    ConnectorLwIp::~ConnectorLwIp()
    {
        if (control != nullptr)
        {
            control->errf = nullptr;    // Avoid tcp_abort triggering callback Err
            tcp_abort(control);
        }
    }

    err_t ConnectorLwIp::StaticConnected(void* arg, tcp_pcb* tpcb, err_t err)
    {
        ConnectorLwIp* connector = reinterpret_cast<ConnectorLwIp*>(arg);
        assert(tpcb == connector->control);
        assert(err == ERR_OK);
        return connector->Connected();
    }

    void ConnectorLwIp::StaticError(void* arg, err_t err)
    {
        reinterpret_cast<ConnectorLwIp*>(arg)->Error(err);
    }

    err_t ConnectorLwIp::Connected()
    {
        infra::SharedPtr<ConnectionLwIp> connection = connectionAllocator.Allocate(control);
        if (connection)
        {
            control = nullptr;
            clientFactory.ConnectionEstablished([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver)
                {
                    connectionObserver->Attach(*connection);
                    connection->SetOwnership(connection, connectionObserver);
                    connectionObserver->Connected();
                }
            });

            infra::WeakPtr<ConnectionLwIp> weakConnection = connection;
            connection = nullptr;
            factory.Remove(*this);
            if (weakConnection.lock())
                return ERR_OK;
            else
                return ERR_ABRT;
        }
        else
        {
            tcp_abort(control);
            control = nullptr;
            clientFactory.ConnectionFailed(ClientConnectionObserverFactory::ConnectFailReason::connectionAllocationFailed);
            factory.Remove(*this);
            return ERR_ABRT;
        }
    }

    void ConnectorLwIp::Error(err_t err)
    {
        control = nullptr;
        clientFactory.ConnectionFailed(ClientConnectionObserverFactory::ConnectFailReason::refused);
        factory.Remove(*this);
    }

    ConnectionFactoryLwIp::ConnectionFactoryLwIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator)
        : listenerAllocator(listenerAllocator)
        , connectors(connectors)
        , connectionAllocator(connectionAllocator)
    {}

    infra::SharedPtr<void> ConnectionFactoryLwIp::Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        return listenerAllocator.Allocate(connectionAllocator, port, factory, versions);
    }

    void ConnectionFactoryLwIp::Connect(ClientConnectionObserverFactory& factory)
    {
        waitingConnectors.push_back(factory);
        TryConnect();
    }

    void ConnectionFactoryLwIp::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        if (waitingConnectors.has_element(factory))
            waitingConnectors.erase(factory);
        else
        {
            for (auto& connector : connectors)
                if (&connector.clientFactory == &factory)
                {
                    connectors.remove(connector);
                    TryConnect();
                    return;
                }

            std::abort();   // Not found
        }
    }

    void ConnectionFactoryLwIp::Remove(ConnectorLwIp& connector)
    {
        connectors.remove(connector);
        TryConnect();
    }

    void ConnectionFactoryLwIp::TryConnect()
    {
        if (!connectors.full() && !waitingConnectors.empty())
        {
            ClientConnectionObserverFactory& factory = waitingConnectors.front();
            waitingConnectors.pop_front();
            connectors.emplace_back(*this, factory, connectionAllocator);
        }
    }
}
