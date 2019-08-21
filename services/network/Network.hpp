#ifndef SERVICES_NETWORK_HPP
#define SERVICES_NETWORK_HPP

#include "services/network/Connection.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Multicast.hpp"
#include "services/network/WiFiNetwork.hpp"

namespace main_
{
    struct Network
    {
        services::ConnectionFactory& connectionFactory;
        services::DatagramFactory& datagramFactory;
        services::Multicast& multicast;
        services::IpConfig ipConfig;
    };
}

#endif
