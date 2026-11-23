#include "lwip/lwip_cpp/IpAddressLwIp.hpp"
#include "lwip/netif.h"
#include <variant>

namespace services
{
    IPv4Address IpAddressLwIp::Convert(const ip4_addr_t& address)
    {
        return {
            ip4_addr1(&address),
            ip4_addr2(&address),
            ip4_addr3(&address),
            ip4_addr4(&address),
        };
    }

    IPv6Address IpAddressLwIp::Convert(const ip6_addr_t& address)
    {
        return {
            IP6_ADDR_BLOCK1(&address),
            IP6_ADDR_BLOCK2(&address),
            IP6_ADDR_BLOCK3(&address),
            IP6_ADDR_BLOCK4(&address),
            IP6_ADDR_BLOCK5(&address),
            IP6_ADDR_BLOCK6(&address),
            IP6_ADDR_BLOCK7(&address),
            IP6_ADDR_BLOCK8(&address)
        };
    }

    IPAddress IpAddressLwIp::Convert(const ip_addr_t& address)
    {
        switch (address.type)
        {
            case IPADDR_TYPE_V4:
                return Convert(address.u_addr.ip4);
            case IPADDR_TYPE_V6:
                return Convert(address.u_addr.ip6);
            default:
                return {};
        }
    }

    ip4_addr_t IpAddressLwIp::ToIp4(const IPv4Address& address)
    {
        ip4_addr_t result;
        IP4_ADDR(&result, address[0], address[1], address[2], address[3]);
        return result;
    }

    ip6_addr_t IpAddressLwIp::ToIp6(const IPv6Address& address)
    {
        ip6_addr_t result;
        IP6_ADDR(&result,
            PP_HTONL(address[1] + (static_cast<uint32_t>(address[0]) << 16)),
            PP_HTONL(address[3] + (static_cast<uint32_t>(address[2]) << 16)),
            PP_HTONL(address[5] + (static_cast<uint32_t>(address[4]) << 16)),
            PP_HTONL(address[7] + (static_cast<uint32_t>(address[6]) << 16)));
        return result;
    }

    ip6_addr_t IpAddressLwIp::ToIp6WithZone(const IPv6Address& address)
    {
        ip6_addr_t result = ToIp6(address);
        ip6_addr_set_zone(&result, netif_default->ip6_addr->u_addr.ip6.zone);
        return result;
    }

    ip_addr_t IpAddressLwIp::ToIpAddr(const IPv4Address& address)
    {
        ip_addr_t result IPADDR4_INIT(0);
        result.u_addr.ip4 = ToIp4(address);
        return result;
    }

    ip_addr_t IpAddressLwIp::ToIpAddr(const IPv6Address& address)
    {
        ip_addr_t result IPADDR6_INIT(0, 0, 0, 0);
        result.u_addr.ip6 = ToIp6(address);
        return result;
    }

    ip_addr_t IpAddressLwIp::ToIpAddr(const IPAddress& address)
    {
        if (std::holds_alternative<IPv4Address>(address))
            return ToIpAddr(std::get<IPv4Address>(address));
        else
            return ToIpAddr(std::get<IPv6Address>(address));
    }

    ip_addr_t IpAddressLwIp::ToIpAddrWithZone(const IPv6Address& address)
    {
        ip_addr_t result IPADDR6_INIT(0, 0, 0, 0);
        result.u_addr.ip6 = ToIp6WithZone(address);
        return result;
    }
}
