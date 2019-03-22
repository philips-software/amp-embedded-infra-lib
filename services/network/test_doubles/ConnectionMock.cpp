#include "services/network/test_doubles/ConnectionMock.hpp"

namespace services
{
    infra::SharedPtr<void> ConnectionFactoryMock::Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        this->serverConnectionObserverFactory = &factory;
        return ListenMock(port, versions);
    }

    void ConnectionFactoryMock::NewConnection(Connection& connection, services::IPAddress address)
    {
        serverConnectionObserverFactory->ConnectionAccepted([&connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            connectionObserver->Attach(connection);
            connection.SetOwnership(nullptr, connectionObserver);
            connectionObserver->Connected();
        }, address);
    }

    ConnectionObserverMock::ConnectionObserverMock(services::Connection& connection)
        : services::ConnectionObserver(connection)
    {}
}
