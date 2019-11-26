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
        MOCK_METHOD2(JoinMulticastGroup, void(infra::SharedPtr<DatagramExchange>, IPv4Address multicastAddress));
        MOCK_METHOD2(LeaveMulticastGroup, void(infra::SharedPtr<DatagramExchange>, IPv4Address multicastAddress));
        MOCK_METHOD2(JoinMulticastGroup, void(infra::SharedPtr<DatagramExchange>, IPv6Address multicastAddress));
        MOCK_METHOD2(LeaveMulticastGroup, void(infra::SharedPtr<DatagramExchange>, IPv6Address multicastAddress));
    };
}

#endif
