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

    void NetworkAdapter::Run()
    {
        network.Run();
    }
}
