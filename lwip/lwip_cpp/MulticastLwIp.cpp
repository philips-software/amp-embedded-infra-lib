#include "lwip/lwip_cpp/MulticastLwIp.hpp"
#include "lwip/igmp.h"
#include "lwip/mld6.h"
#include "lwip/netif.h"
#include <cassert>

namespace services
{
    void MulticastLwIp::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        ip4_addr_t groupAddress;
        IP4_ADDR(&groupAddress, multicastAddress[0], multicastAddress[1], multicastAddress[2], multicastAddress[3]);
        err_t result = igmp_joingroup(IPADDR_ANY, &groupAddress);
        assert(result == ERR_OK);
    }

    void MulticastLwIp::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        ip4_addr_t groupAddress;
        IP4_ADDR(&groupAddress, multicastAddress[0], multicastAddress[1], multicastAddress[2], multicastAddress[3]);
        err_t result = igmp_leavegroup(IPADDR_ANY, &groupAddress);
        assert(result == ERR_OK);
    }

    void MulticastLwIp::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress)
    {
        ip6_addr_t groupAddress;
        ip6_addr_t ip6Any;
        IP6_ADDR(&groupAddress, PP_HTONL(multicastAddress[1] + (static_cast<uint32_t>(multicastAddress[0]) << 16)), PP_HTONL(multicastAddress[3] + (static_cast<uint32_t>(multicastAddress[2]) << 16)), PP_HTONL(multicastAddress[5] + (static_cast<uint32_t>(multicastAddress[4]) << 16)), PP_HTONL(multicastAddress[7] + (static_cast<uint32_t>(multicastAddress[6]) << 16)));
        ip6_addr_set_any(&ip6Any);
        err_t result = mld6_joingroup(&ip6Any, &groupAddress);
        assert(result == ERR_OK);
    }

    void MulticastLwIp::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress)
    {
        ip6_addr_t groupAddress;
        ip6_addr_t ip6Any;
        IP6_ADDR(&groupAddress, PP_HTONL(multicastAddress[1] + (static_cast<uint32_t>(multicastAddress[0]) << 16)), PP_HTONL(multicastAddress[3] + (static_cast<uint32_t>(multicastAddress[2]) << 16)), PP_HTONL(multicastAddress[5] + (static_cast<uint32_t>(multicastAddress[4]) << 16)), PP_HTONL(multicastAddress[7] + (static_cast<uint32_t>(multicastAddress[6]) << 16)));
        ip6_addr_set_any(&ip6Any);
        err_t result = mld6_leavegroup(&ip6Any, &groupAddress);
        assert(result == ERR_OK);
    }
}
