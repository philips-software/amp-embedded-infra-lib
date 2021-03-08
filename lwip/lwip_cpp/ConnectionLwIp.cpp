#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "lwip/lwip_cpp/ConnectionLwIp.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace services
{
    namespace
    {
        static tcp_ext_arg_callbacks emptyCallbacks{};
    }

    infra::BoundedList<std::array<uint8_t, TCP_MSS>>::WithMaxSize<tcpSndQueueLen> ConnectionLwIp::sendMemoryPool;
    infra::IntrusiveList<ConnectionLwIp> ConnectionLwIp::sendMemoryPoolWaiting;

    ConnectionLwIp::ConnectionLwIp(ConnectionFactoryLwIp& factory, tcp_pcb* control)
        : factory(factory)
        , control(control)
    {
        factory.connections.push_front(*this);
        assert(control != nullptr);
        tcp_arg(control, this);
        tcp_recv(control, &ConnectionLwIp::Recv);
        tcp_err(control, &ConnectionLwIp::Err);
        tcp_sent(control, &ConnectionLwIp::Sent);
        tcp_nagle_disable(control);

        callbacks.destroy = &ConnectionLwIp::Destroy;
        callbacks.passive_open = 0;
        tcp_ext_arg_set_callbacks(control, 0, &callbacks);
        tcp_ext_arg_set(control, 0, this);
    }

    ConnectionLwIp::~ConnectionLwIp()
    {
        factory.connections.erase_slow(*this);

        while (!sendBuffers.empty())
        {
            RemoveFromPool(sendBuffers.front());
            sendBuffers.pop_front();
        }

        if (!sendBufferForStream.empty())
            RemoveFromPool(sendBufferForStream);

        if (!sendBuffer.empty())
            RemoveFromPool(sendBuffer);

        if (sendMemoryPoolWaiting.has_element(*this))
            sendMemoryPoolWaiting.erase(*this);

        if (receivedData != nullptr)
            pbuf_free(receivedData);

        if (control)
            AbortControl();
    }

    void ConnectionLwIp::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        assert(sendSize != 0 && sendSize <= MaxSendStreamSize());
        really_assert(control != nullptr);
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
        tcp_ext_arg_set_callbacks(control, 0, &emptyCallbacks);
        tcp_arg(control, nullptr);
        tcp_recv(control, nullptr);
        tcp_err(control, nullptr);
        tcp_sent(control, nullptr);

        err_t result = tcp_close(control);
        if (result != ERR_OK)
            AbortAndDestroy();
        else
        {
            control = nullptr;  // tcp_close frees the pcb
            ResetOwnership();
        }
    }

    void ConnectionLwIp::AbortAndDestroy()
    {
        if (control)
        {
            AbortControl();
            ResetOwnership();
        }
    }

    void ConnectionLwIp::Attach(const infra::SharedPtr<ConnectionObserver>& observer)
    {
        Connection::Attach(observer);

        if (receivedData != nullptr)
            observer->DataReceived();
    }

    void ConnectionLwIp::SetSelfOwnership(const infra::SharedPtr<ConnectionObserver>& observer)
    {
        self = SharedFromThis();
    }

    void ConnectionLwIp::ResetOwnership()
    {
        if (IsAttached())
            Detach();
        self = nullptr;
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

    bool ConnectionLwIp::PendingSend() const
    {
        return !(sendBuffers.empty() && sendBufferForStream.empty() && sendBuffer.empty());
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
            if (sendMemoryPoolWaiting.has_element(*this))
                sendMemoryPoolWaiting.erase(*this);

            sendMemoryPool.emplace_back();
            sendBufferForStream = infra::Head(infra::ByteRange(sendMemoryPool.back()), requestedSendSize);
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionLwIp>& self)
            {
                infra::SharedPtr<infra::StreamWriter> stream = self->streamWriter.Emplace(*self, self->sendBufferForStream);
                self->sendBufferForStream = infra::ByteRange();
                self->Observer().SendStreamAvailable(std::move(stream));
            }, SharedFromThis());

            requestedSendSize = 0;
        }
        else if (sendMemoryPool.full())
        {
            if (!sendMemoryPoolWaiting.has_element(*this))
                sendMemoryPoolWaiting.push_back(*this);
        }
    }

    void ConnectionLwIp::AbortControl()
    {
        tcp_ext_arg_set_callbacks(control, 0, &emptyCallbacks);

        tcp_arg(control, nullptr);
        tcp_recv(control, nullptr);
        tcp_err(control, nullptr);
        tcp_sent(control, nullptr);
        tcp_abort(control);
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

    void ConnectionLwIp::Destroy(u8_t id, void* data)
    {
        static_cast<ConnectionLwIp*>(data)->Destroy();
    }

    err_t ConnectionLwIp::Recv(pbuf* p, err_t err)
    {
        if (p != nullptr)
        {
            if (receivedData == nullptr)
                receivedData = p;
            else
                pbuf_cat(receivedData, p);

            if (!dataReceivedScheduled && IsAttached())
            {
                dataReceivedScheduled = true;
                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionLwIp>& self)
                {
                    self->dataReceivedScheduled = false;
                    self->Observer().DataReceived();
                }, SharedFromThis());
            }
            return ERR_OK;
        }
        else
        {
            ResetOwnership();
            return ERR_ABRT;
        }
    }

    void ConnectionLwIp::Err(err_t err)
    {
        services::GlobalTracer().Trace() << "ConnectionLwIp::Err received err " << err;
        assert(err == ERR_RST || err == ERR_CLSD || err == ERR_ABRT);
        control = nullptr;  // When Err is received, the pcb has already been freed
        ResetOwnership();
    }

    err_t ConnectionLwIp::Sent(std::uint16_t len)
    {
        while (len != 0)
        {
            really_assert(!sendBuffers.empty());
            if (sendBuffers.front().size() <= len)
            {
                len -= static_cast<uint16_t>(sendBuffers.front().size());

                RemoveFromPool(sendBuffers.front());
                sendBuffers.pop_front();
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

    void ConnectionLwIp::Destroy()
    {
        // Invoked when LightweightIP destroyed the pcb already
        control = nullptr;
        ResetOwnership();
    }

    void ConnectionLwIp::RemoveFromPool(infra::ConstByteRange range)
    {
        auto size = sendMemoryPool.size();
        for (auto& r : sendMemoryPool)
            if (infra::MakeRange(r).contains(range.begin()))
                sendMemoryPool.remove(r);
        really_assert(size == sendMemoryPool.size() + 1);

        if (!sendMemoryPoolWaiting.empty())
            sendMemoryPoolWaiting.front().TryAllocateSendStream();
    }

    ConnectionLwIp::StreamWriterLwIp::StreamWriterLwIp(ConnectionLwIp& connection, infra::ByteRange sendBuffer)
        : infra::ByteOutputStreamWriter(sendBuffer)
        , connection(connection)
    {}

    ConnectionLwIp::StreamWriterLwIp::~StreamWriterLwIp()
    {
        if (!Processed().empty() && connection.control != nullptr)
            connection.SendBuffer(Processed());
        else if (!Processed().empty())
            connection.RemoveFromPool(Processed());
        else
            connection.RemoveFromPool(Remaining());
    }

    ConnectionLwIp::StreamReaderLwIp::StreamReaderLwIp(ConnectionLwIp& connection)
        : connection(connection)
        , currentPbuf(connection.receivedData)
        , offsetInCurrentPbuf(connection.consumed)
    {}

    void ConnectionLwIp::StreamReaderLwIp::ConsumeRead()
    {
        if (connection.control != nullptr)
            tcp_recved(connection.control, static_cast<uint16_t>(offset));

        offset += connection.consumed;
        while (offset != 0 && offset >= connection.receivedData->len)
        {
            offset -= connection.receivedData->len;
            auto newReceivedData = connection.receivedData->next;
            pbuf_ref(newReceivedData);
            pbuf_dechain(connection.receivedData);
            pbuf_free(connection.receivedData);
            connection.receivedData = newReceivedData;
            connection.consumed = 0;
        }

        connection.consumed = offset;
        offset = 0;

        currentPbuf = connection.receivedData;
        offsetInCurrentPbuf = connection.consumed;
    }

    void ConnectionLwIp::StreamReaderLwIp::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        while (!range.empty())
        {
            auto chunk = ExtractContiguousRange(range.size());
            if (chunk.empty())
                break;

            infra::Copy(chunk, infra::Head(range, chunk.size()));
            range.pop_front(chunk.size());
        }

        errorPolicy.ReportResult(range.empty());
    }

    uint8_t ConnectionLwIp::StreamReaderLwIp::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        auto range = PeekContiguousRange(0);
        errorPolicy.ReportResult(!range.empty());
        if (!range.empty())
            return range.front();
        else
            return 0;
    }

    infra::ConstByteRange ConnectionLwIp::StreamReaderLwIp::ExtractContiguousRange(std::size_t max)
    {
        auto chunk = infra::Head(PeekContiguousRange(0), max);

        offset += chunk.size();
        offsetInCurrentPbuf += chunk.size();

        if (currentPbuf != nullptr && offsetInCurrentPbuf == currentPbuf->len)
        {
            currentPbuf = currentPbuf->next;
            offsetInCurrentPbuf = 0;
        }

        return chunk;
    }

    infra::ConstByteRange ConnectionLwIp::StreamReaderLwIp::PeekContiguousRange(std::size_t start)
    {
        auto peekOffset = offsetInCurrentPbuf;
        auto peekPbuf = currentPbuf;

        while (start != 0 && peekPbuf != nullptr)
        {
            auto peekDelta = std::min(start, peekPbuf->len - peekOffset);
            peekOffset += peekDelta;
            start -= peekDelta;
            if (peekOffset == peekPbuf->len)
            {
                peekOffset = 0;
                peekPbuf = peekPbuf->next;
            }
        }

        if (peekPbuf == nullptr)
            return infra::ConstByteRange();
        else
            return infra::ConstByteRange(reinterpret_cast<const uint8_t*>(peekPbuf->payload) + peekOffset, reinterpret_cast<const uint8_t*>(peekPbuf->payload) + peekPbuf->len);
    }

    bool ConnectionLwIp::StreamReaderLwIp::Empty() const
    {
        return Available() == 0;
    }

    std::size_t ConnectionLwIp::StreamReaderLwIp::Available() const
    {
        if (connection.receivedData != nullptr)
            return connection.receivedData->tot_len - connection.consumed - offset;
        else
            return 0;
    }

    std::size_t ConnectionLwIp::StreamReaderLwIp::ConstructSaveMarker() const
    {
        return offset;
    }

    void ConnectionLwIp::StreamReaderLwIp::Rewind(std::size_t marker)
    {
        offset = 0;
        currentPbuf = connection.receivedData;
        offsetInCurrentPbuf = connection.consumed;

        while (marker != 0)
        {
            auto delta = std::min(marker, currentPbuf->len - offsetInCurrentPbuf);
            offsetInCurrentPbuf += delta;
            marker -= delta;
            if (offsetInCurrentPbuf == currentPbuf->len)
            {
                offsetInCurrentPbuf = 0;
                currentPbuf = currentPbuf->next;
            }
        }
    }

    ListenerLwIp::ListenerLwIp(AllocatorConnectionLwIp& allocator, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions, ConnectionFactoryLwIp& connectionFactory)
        : allocator(allocator)
        , factory(factory)
        , access([this]() { TryProcessBacklog(); })
        , connectionFactory(connectionFactory)
    {
        tcp_pcb* pcb = tcp_new();
        assert(pcb != nullptr);
        ip_set_option(pcb, SOF_REUSEADDR);
        if (versions == IPVersions::both)
            err_t err = tcp_bind(pcb, IP_ANY_TYPE, port);
        else if (versions == IPVersions::ipv4)
            err_t err = tcp_bind(pcb, IP4_ADDR_ANY, port);
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
        services::GlobalTracer().Trace() << "ListenerLwIp::Accept accepted new connection";
        PurgeBacklog();
        if (!backlog.full())
        {
            infra::SharedPtr<ConnectionLwIp> connection = allocator.Allocate(connectionFactory, newPcb);
            if (connection)
            {
                backlog.push_back(connection);
                return ProcessBacklog();
            }
            else
            {
                services::GlobalTracer().Trace() << "ListenerLwIp::ProcessBacklog connection allocation failed";
                tcp_abort(newPcb);
                return ERR_ABRT;
            }
        }
        else
        {
            services::GlobalTracer().Trace() << "ListenerLwIp::Accept backlog is full";
            tcp_abort(newPcb);
            return ERR_ABRT;
        }
    }

    void ListenerLwIp::TryProcessBacklog()
    {
        auto weakSelf = infra::WeakPtr<ListenerLwIp>(this->self);
        this->self = nullptr;

        if (weakSelf.lock())
            ProcessBacklog();
    }

    err_t ListenerLwIp::ProcessBacklog()
    {
        PurgeBacklog();
        if (!backlog.empty() && !access.Referenced())
        {
            services::GlobalTracer().Trace() << "ListenerLwIp::ProcessBacklog processing new connection";

            this->self = SharedFromThis();
            auto self = access.MakeShared(*this);

            auto connection = self->backlog.front();
            factory.ConnectionAccepted([self](infra::SharedPtr<services::ConnectionObserver> connectionObserver) {
                auto connection = self->backlog.front();
                self->backlog.pop_front();

                if (connectionObserver && connection->control != nullptr)
                {
                    connection->SetSelfOwnership(connectionObserver);
                    connection->Attach(connectionObserver);
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
            return ERR_OK;
    }

    void ListenerLwIp::PurgeBacklog()
    {
        for (auto i = backlog.begin(); i != backlog.end();)
        {
            if ((*i)->control == nullptr)
            {
                services::GlobalTracer().Trace() << "ListenerLwIp::PurgeBacklog removing closed connection";
                backlog.erase(i++);
            }
            else
                ++i;
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
            AbortControl();
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
        services::GlobalTracer().Trace() << "ConnectorLwIp::Connected connection established";
        infra::SharedPtr<ConnectionLwIp> connection = connectionAllocator.Allocate(factory, control);
        if (connection)
        {
            ResetControl();
            clientFactory.ConnectionEstablished([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver && connection->control != nullptr)
                {
                    connection->SetSelfOwnership(connectionObserver);
                    connection->Attach(connectionObserver);
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
            services::GlobalTracer().Trace() << "ConnectorLwIp::Connected connection allocation failed";
            AbortControl();
            clientFactory.ConnectionFailed(ClientConnectionObserverFactory::ConnectFailReason::connectionAllocationFailed);
            factory.Remove(*this);
            return ERR_ABRT;
        }
    }

    void ConnectorLwIp::Error(err_t err)
    {
        ResetControl();
        clientFactory.ConnectionFailed(ClientConnectionObserverFactory::ConnectFailReason::refused);
        factory.Remove(*this);
    }

    void ConnectorLwIp::ResetControl()
    {
        control->errf = nullptr;
        control->connected = nullptr;
        control = nullptr;
    }

    void ConnectorLwIp::AbortControl()
    {
        control->errf = nullptr;    // Avoid tcp_abort triggering callback Err
        control->connected = nullptr;
        tcp_abort(control);
        control = nullptr;
    }

    ConnectionFactoryLwIp::ConnectionFactoryLwIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator)
        : listenerAllocator(listenerAllocator)
        , connectors(connectors)
        , connectionAllocator(connectionAllocator)
    {}

    infra::SharedPtr<void> ConnectionFactoryLwIp::Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        return listenerAllocator.Allocate(connectionAllocator, port, factory, versions, *this);
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

    bool ConnectionFactoryLwIp::PendingSend() const
    {
        return std::any_of(connections.begin(), connections.end(), [](auto& c)
        {
            return c.PendingSend();
        });
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
