#include "services/network_instantiations/NetworkAdapter.hpp"

namespace main_
{
    services::ConnectionFactory& NetworkAdapter::ConnectionFactory()
    {
        return network;
    }

    services::DatagramFactory& NetworkAdapter::DatagramFactory()
    {
        return network;
    }

    services::Multicast& NetworkAdapter::Multicast()
    {
        return network;
    }

    services::NameResolver& NetworkAdapter::NameResolver()
    {
        return nameResolver;
    }

    services::EventDispatcherWithNetwork& NetworkAdapter::EventDispatcher()
    {
        return network;
    }

    void NetworkAdapter::Run()
    {
        network.Run();
    }

    void NetworkAdapter::ExecuteUntil(const infra::Function<bool()>& predicate)
    {
        network.ExecuteUntil(predicate);
    }

    bool NetworkAdapter::NetworkActivity() const
    {
        return network.OpenConnections() || nameResolver.Active();
    }
}
