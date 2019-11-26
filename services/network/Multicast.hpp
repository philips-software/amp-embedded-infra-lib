#ifndef SERVICES_MULTICAST_HPP
#define SERVICES_MULTICAST_HPP

#include "infra/util/SharedPtr.hpp"
#include "services/network/Address.hpp"

namespace services
{
    class DatagramExchange;

    class Multicast
    {
    protected:
        Multicast() = default;
        Multicast(const Multicast& other) = delete;
        Multicast& operator=(const Multicast& other) = delete;
        virtual ~Multicast() = default;

    public:
        virtual void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) = 0;
        virtual void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) = 0;
        virtual void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) = 0;
        virtual void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) = 0;
    };
}

#endif
