#ifndef SERVICES_MULTICAST_HPP
#define SERVICES_MULTICAST_HPP

#include "services/network/Address.hpp"

namespace services
{
    class Multicast
    {
    protected:
        Multicast() = default;
        Multicast(const Multicast& other) = delete;
        Multicast& operator=(const Multicast& other) = delete;
        virtual ~Multicast() = default;

    public:
        virtual void JoinMulticastGroup(IPv4Address multicastAddress) = 0;
        virtual void LeaveMulticastGroup(IPv4Address multicastAddress) = 0;
        virtual void JoinMulticastGroup(IPv6Address multicastAddress) = 0;
        virtual void LeaveMulticastGroup(IPv6Address multicastAddress) = 0;
    };
}

#endif
