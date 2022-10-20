#ifndef NETWORK_ADDRESS_MOCK_HPP
#define NETWORK_ADDRESS_MOCK_HPP

#include "services/network/Address.hpp"
#include "gmock/gmock.h"

namespace services
{
    class IPv4InfoMock
        : public IPv4Info
    {
    public:
        MOCK_CONST_METHOD0(GetIPv4Address, IPv4Address());
        MOCK_CONST_METHOD0(GetIPv4InterfaceAddresses, IPv4InterfaceAddresses());
    };

    class IPv6InfoMock
        : public IPv6Info
    {
    public:
        MOCK_CONST_METHOD0(LinkLocalAddress, IPv6Address());
    };

    class IPInfoMock
        : public IPInfo
    {
    public:
        MOCK_CONST_METHOD0(GetIPv4Address, IPv4Address());
        MOCK_CONST_METHOD0(GetIPv4InterfaceAddresses, IPv4InterfaceAddresses());
        MOCK_CONST_METHOD0(LinkLocalAddress, IPv6Address());
    };
}

#endif
