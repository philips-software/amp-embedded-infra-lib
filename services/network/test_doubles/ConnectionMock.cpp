#include "services/network/test_doubles/ConnectionMock.hpp"

namespace services
{
    void ConnectionFactoryMock::NewConnection(ServerConnectionObserverFactory& serverConnectionObserverFactory, Connection& connection, services::IPAddress address)
    {
        serverConnectionObserverFactory.ConnectionAccepted([&connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            connectionObserver->Attach(connection);
            connection.SetOwnership(nullptr, connectionObserver);
            connectionObserver->Connected();
        }, address);
    }

    void ConnectionFactoryMock::NewConnection(ServerConnectionObserverFactory& serverConnectionObserverFactory, infra::SharedPtr<Connection> connection, services::IPAddress address)
    {
        serverConnectionObserverFactory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            connectionObserver->Attach(*connection);
            connection->SetOwnership(connection, connectionObserver);
            connectionObserver->Connected();
        }, address);
    }

    ConnectionObserverMock::ConnectionObserverMock(services::Connection& connection)
        : services::ConnectionObserver(connection)
    {}
}
