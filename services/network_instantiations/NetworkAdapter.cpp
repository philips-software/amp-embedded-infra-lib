#include "services/network_instantiations/NetworkAdapter.hpp"

namespace main_
{
    services::ConnectionFactory& NetworkAdapter::ConnectionFactory()
    {
        return network;
    }

    services::ConnectionFactoryWithNameResolver& NetworkAdapter::ConnectionFactoryWithNameResolver()
    {
        return connectionFactoryWithNameResolver;
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
}
