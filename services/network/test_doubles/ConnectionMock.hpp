#ifndef NETWORK_CONNECTION_MOCK_HPP
#define NETWORK_CONNECTION_MOCK_HPP

#include "gmock/gmock.h"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include <vector>

namespace services
{
    //TICS -INT#002: A mock or stub may have public data
    //TICS -INT#027: MOCK_METHOD can't add 'virtual' to its signature
    class ConnectionMock
        : public services::Connection
    {
    public:
        MOCK_METHOD1(RequestSendStream, void(std::size_t sendSize));
        MOCK_CONST_METHOD0(MaxSendStreamSize, std::size_t());
        MOCK_METHOD0(ReceiveStream, infra::SharedPtr<infra::StreamReaderWithRewinding>());
        MOCK_METHOD0(AckReceived, void());
        MOCK_METHOD0(CloseAndDestroy, void());
        MOCK_METHOD0(AbortAndDestroy, void());
        MOCK_CONST_METHOD0(Ipv4Address, IPv4Address());
    };

    class ConnectionWithHostnameMock
        : public services::ConnectionWithHostname
    {
    public:
        MOCK_METHOD1(RequestSendStream, void(std::size_t sendSize));
        MOCK_CONST_METHOD0(MaxSendStreamSize, std::size_t());
        MOCK_METHOD0(ReceiveStream, infra::SharedPtr<infra::StreamReaderWithRewinding>());
        MOCK_METHOD0(AckReceived, void());
        MOCK_METHOD0(CloseAndDestroy, void());
        MOCK_METHOD0(AbortAndDestroy, void());
        MOCK_CONST_METHOD0(Ipv4Address, IPv4Address());
        MOCK_METHOD1(SetHostname, void(infra::BoundedConstString hostname));
    };

    class ConnectionObserverMock
        : public services::ConnectionObserver
    {
    public:
        ConnectionObserverMock() = default;
        explicit ConnectionObserverMock(services::Connection& connection);

        using services::ConnectionObserver::Subject;

        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override { SendStreamAvailable(writer); }
        MOCK_METHOD1(SendStreamAvailable, void(infra::SharedPtr<infra::StreamWriter> writer));
        MOCK_METHOD0(DataReceived, void());
    };

    class ConnectionObserverFullMock
        : public ConnectionObserver
    {
    public:
        using ConnectionObserver::ConnectionObserver;

        using services::ConnectionObserver::Subject;

        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override { SendStreamAvailable(writer); }
        MOCK_METHOD1(SendStreamAvailable, void(infra::SharedPtr<infra::StreamWriter> writer));
        MOCK_METHOD0(DataReceived, void());
        MOCK_METHOD0(Connected, void());
        MOCK_METHOD0(ClosingConnection, void());
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(Abort, void());
    };

    class ConnectionFactoryMock
        : public services::ConnectionFactory
    {
    public:
        MOCK_METHOD3(Listen, infra::SharedPtr<void>(uint16_t, ServerConnectionObserverFactory& factory, IPVersions));
        MOCK_METHOD1(Connect, void(ClientConnectionObserverFactory& factory));
        MOCK_METHOD1(CancelConnect, void(ClientConnectionObserverFactory& factory));

        void NewConnection(ServerConnectionObserverFactory& serverConnectionObserverFactory, Connection& connection, services::IPAddress address);
        void NewConnection(ServerConnectionObserverFactory& serverConnectionObserverFactory, infra::SharedPtr<Connection> connection, services::IPAddress address);
    };

    class ServerConnectionObserverFactoryMock
        : public services::ServerConnectionObserverFactory
    {
    public:
        void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address) { ConnectionAcceptedMock(createdObserver.Clone(), address); }
        MOCK_METHOD2(ConnectionAcceptedMock, void(infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address));
    };

    class ClientConnectionObserverFactoryMock
        : public services::ClientConnectionObserverFactory
    {
    public:
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) { ConnectionEstablishedMock(createdObserver.Clone()); }
        MOCK_CONST_METHOD0(Address, services::IPAddress());
        MOCK_CONST_METHOD0(Port, uint16_t());
        MOCK_METHOD1(ConnectionEstablishedMock, void(infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver));
        MOCK_METHOD1(ConnectionFailed, void(ConnectFailReason reason));
        MOCK_METHOD0(Destructor, void());

        virtual ~ClientConnectionObserverFactoryMock() { Destructor(); }
    };

    class ConnectionFactoryWithNameResolverMock
        : public ConnectionFactoryWithNameResolver
    {
    public:
        MOCK_METHOD1(Connect, void(ClientConnectionObserverFactoryWithNameResolver& factory));
        MOCK_METHOD1(CancelConnect, void(ClientConnectionObserverFactoryWithNameResolver& factory));
    };
}

#endif
