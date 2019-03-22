#ifndef NETWORK_MULTICAST_MOCK_HPP
#define NETWORK_MULTICAST_MOCK_HPP

#include "gmock/gmock.h"
#include "services/network/Multicast.hpp"

namespace services
{
    class MulticastMock
        : public Multicast
    {
    public:
        MOCK_METHOD1(JoinMulticastGroup, void(IPv4Address multicastAddress));
        MOCK_METHOD1(LeaveMulticastGroup, void(IPv4Address multicastAddress));
        MOCK_METHOD1(JoinMulticastGroup, void(IPv6Address multicastAddress));
        MOCK_METHOD1(LeaveMulticastGroup, void(IPv6Address multicastAddress));
    };
}

#endif
