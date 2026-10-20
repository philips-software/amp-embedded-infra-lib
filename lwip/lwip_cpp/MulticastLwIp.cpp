#include "lwip/lwip_cpp/MulticastLwIp.hpp"
#include "lwip/igmp.h"
#include "lwip/lwip_cpp/IpAddressLwIp.hpp"
#include "lwip/mld6.h"
#include "lwip/netif.h"
#include <cassert>

namespace services
{
    void MulticastLwIp::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        ip4_addr_t groupAddress = IpAddressLwIp::ToIp4(multicastAddress);
        err_t result = igmp_joingroup(&IP4_ADDR_ANY->u_addr.ip4, &groupAddress);
        assert(result == ERR_OK);
    }

    void MulticastLwIp::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        ip4_addr_t groupAddress = IpAddressLwIp::ToIp4(multicastAddress);
        err_t result = igmp_leavegroup(&IP4_ADDR_ANY->u_addr.ip4, &groupAddress);
        assert(result == ERR_OK);
    }

    void MulticastLwIp::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress)
    {
        ip6_addr_t groupAddress = IpAddressLwIp::ToIp6(multicastAddress);
        err_t result = mld6_joingroup(&IP6_ADDR_ANY->u_addr.ip6, &groupAddress);
        assert(result == ERR_OK);
    }

    void MulticastLwIp::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress)
    {
        ip6_addr_t groupAddress = IpAddressLwIp::ToIp6(multicastAddress);
        err_t result = mld6_leavegroup(&IP6_ADDR_ANY->u_addr.ip6, &groupAddress);
        assert(result == ERR_OK);
    }
}
