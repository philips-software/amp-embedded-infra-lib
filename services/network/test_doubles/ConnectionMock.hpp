#ifndef NETWORK_CONNECTION_MOCK_HPP
#define NETWORK_CONNECTION_MOCK_HPP

#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "gmock/gmock.h"
#include <vector>

namespace services
{
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

        void SetOwnership(const infra::SharedPtr<Connection>& owner, const infra::SharedPtr<ConnectionObserver>& observer)
        {
            self = owner;
        }

        void ResetOwnership()
        {
            Detach();
            self = nullptr;
        }

    private:
        infra::SharedPtr<Connection> self;
    };

    class ConnectionObserverMock
        : public services::ConnectionObserver
    {
    public:
        MOCK_METHOD1(SendStreamAvailable, void(infra::SharedPtr<infra::StreamWriter>&& writer));
        MOCK_METHOD0(DataReceived, void());
    };

    class ConnectionObserverFullMock
        : public ConnectionObserver
    {
    public:
        using ConnectionObserver::ConnectionObserver;

        MOCK_METHOD1(SendStreamAvailable, void(infra::SharedPtr<infra::StreamWriter>&& writer));
        MOCK_METHOD0(DataReceived, void());
        MOCK_METHOD0(Attached, void());
        MOCK_METHOD0(Detaching, void());
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
    };

    class ServerConnectionObserverFactoryMock
        : public services::ServerConnectionObserverFactory
    {
    public:
        MOCK_METHOD2(ConnectionAccepted, void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address));
    };

    class ClientConnectionObserverFactoryWithNameResolverMock
        : public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        MOCK_CONST_METHOD0(Hostname, infra::BoundedConstString());
        MOCK_CONST_METHOD0(Port, uint16_t());
        MOCK_METHOD1(ConnectionEstablished, void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver));
        MOCK_METHOD1(ConnectionFailed, void(ConnectFailReason reason));
        MOCK_METHOD1(ToString, const char*(ConnectFailReason reason));
    };

    class ClientConnectionObserverFactoryMock
        : public services::ClientConnectionObserverFactory
    {
    public:
        MOCK_CONST_METHOD0(Address, services::IPAddress());
        MOCK_CONST_METHOD0(Port, uint16_t());
        MOCK_METHOD1(ConnectionEstablished, void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver));
        MOCK_METHOD1(ConnectionFailed, void(ConnectFailReason reason));
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
