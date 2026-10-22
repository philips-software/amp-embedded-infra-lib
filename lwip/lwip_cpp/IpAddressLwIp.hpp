#ifndef LWIP_IP_ADDRESS_LW_IP_HPP
#define LWIP_IP_ADDRESS_LW_IP_HPP

#include "lwip/ip_addr.h"
#include "services/network/Address.hpp"

namespace services
{
    class IpAddressLwIp
    {
    public:
        IpAddressLwIp() = delete;

        static IPv4Address Convert(const ip4_addr_t& address);
        static IPv6Address Convert(const ip6_addr_t& address);
        static IPAddress Convert(const ip_addr_t& address);

        static ip4_addr_t ToIp4(const IPv4Address& address);
        static ip6_addr_t ToIp6(const IPv6Address& address);
        static ip6_addr_t ToIp6WithZone(const IPv6Address& address);
        static ip_addr_t ToIpAddr(const IPv4Address& address);
        static ip_addr_t ToIpAddr(const IPv6Address& address);
        static ip_addr_t ToIpAddr(const IPAddress& address);
        static ip_addr_t ToIpAddrWithZone(const IPv6Address& address);
    };
}

#endif
