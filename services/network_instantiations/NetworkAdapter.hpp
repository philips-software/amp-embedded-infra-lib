#ifndef SERVICES_NETWORK_INSTANTIATIONS_HPP
#define SERVICES_NETWORK_INSTANTIATIONS_HPP

#include "services/network/Connection.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Multicast.hpp"
#include "services/network/NameResolver.hpp"

#ifdef EMIL_NETWORK_WIN
#include "services/network_win/EventDispatcherWithNetwork.hpp"
#include "services/network_win/NameLookup.hpp"
#endif

#ifdef EMIL_NETWORK_BSD
#include "services/network_bsd/EventDispatcherWithNetwork.hpp"
#include "services/network_bsd/NameLookup.hpp"
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
