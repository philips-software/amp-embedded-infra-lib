#ifndef LWIP_MULTICAST_LW_IP_HPP
#define LWIP_MULTICAST_LW_IP_HPP

#include "services/network/Multicast.hpp"

namespace services
{
    class MulticastLwIp
        : public Multicast
    {
    public:
        void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;
        void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;
    };
}

#endif
