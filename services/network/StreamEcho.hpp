#ifndef SERVICES_STREAM_ECHO_HPP
#define SERVICES_STREAM_ECHO_HPP

#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class StreamEchoConnection
        : public services::ConnectionObserver
    {
    public:
        StreamEchoConnection() = default;

        void SendInitial();

        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    private:
        void TryRequestSendStream();

    private:
        std::size_t requestedSize = 0;
    };

    using AllocatorStreamEchoConnection = infra::SharedObjectAllocator<StreamEchoConnection, void()>;

    class StreamEchoServer
        : public services::ServerConnectionObserverFactory
    {
    public:
        template<std::size_t MaxConnections>
        using WithMaxConnections = infra::WithStorage<StreamEchoServer, AllocatorStreamEchoConnection::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

        StreamEchoServer(AllocatorStreamEchoConnection& allocator, services::ConnectionFactory& listenerFactory, uint16_t port, services::IPVersions versions = services::IPVersions::both);

    protected:
        virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address) override;

    private:
        AllocatorStreamEchoConnection& allocator;
        infra::SharedPtr<void> listenPort;
    };
}

#endif
