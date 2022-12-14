#ifndef SERVICES_NETWORK_INSTANTIATIONS_HPP
#define SERVICES_NETWORK_INSTANTIATIONS_HPP

#include "services/network_instantiations/NameLookup.hpp"

#ifdef EMIL_NETWORK_WIN
#include "services/network_instantiations/EventDispatcherWithNetworkWin.hpp"
#endif

#ifdef EMIL_NETWORK_BSD
#include "services/network_instantiations/EventDispatcherWithNetworkBsd.hpp"
#endif

namespace main_
{
    class NetworkAdapter
    {
    public:
        services::ConnectionFactory& ConnectionFactory();
        services::DatagramFactory& DatagramFactory();
        services::Multicast& Multicast();
        services::NameResolver& NameResolver();

        void Run();

    private:
        services::EventDispatcherWithNetwork network;
        services::NameLookup nameResolver;
    };
}

#endif
