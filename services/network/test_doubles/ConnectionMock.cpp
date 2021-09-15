#include "services/network/test_doubles/ConnectionMock.hpp"

namespace services
{
    void ConnectionFactoryMock::NewConnection(ServerConnectionObserverFactory& serverConnectionObserverFactory, Connection& connection, services::IPAddress address)
    {
        serverConnectionObserverFactory.ConnectionAccepted([&connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver) {
            connection.Attach(connectionObserver);
        },
            address);
    }
}
