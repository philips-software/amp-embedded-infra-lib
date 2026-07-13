#include "lwip/lwip_cpp/MacResolverLwIp.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"
#include "lwip/nd6.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "netif/etharp.h"
#include <algorithm>

namespace services
{
    namespace
    {
        ip6_addr_t ToLwipIp6(const services::IPv6Address& address)
        {
            ip6_addr_t target;
            IP6_ADDR(&target,
                PP_HTONL(address[1] + (static_cast<uint32_t>(address[0]) << 16)),
                PP_HTONL(address[3] + (static_cast<uint32_t>(address[2]) << 16)),
                PP_HTONL(address[5] + (static_cast<uint32_t>(address[4]) << 16)),
                PP_HTONL(address[7] + (static_cast<uint32_t>(address[6]) << 16)));
            ip6_addr_set_zone(&target, netif_default->ip6_addr->u_addr.ip6.zone);
            return target;
        }
    }

    MacResolverRetryHelper::MacResolverRetryHelper(uint8_t retries, infra::Duration retryInterval, LookupFunction lookup, SendRequestFunction sendRequest)
        : retries(retries)
        , lookup(lookup)
        , sendRequest(sendRequest)
        , retryTimer(retryInterval)
    {}

    void MacResolverRetryHelper::Resolve(const IPAddress& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone)
    {
        really_assert(!onDone);

        if (auto mac = lookup(address); mac)
        {
            onResolveDone(mac);
            return;
        }

        pendingAddress = address;
        onDone = onResolveDone;
        retriesLeft = retries;

        sendRequest(address);
        retryTimer.Start(infra::FailureType::intermittentFailure, [this]
            {
                OnRetry();
            });
    }

    void MacResolverRetryHelper::OnRetry()
    {
        if (auto mac = lookup(pendingAddress); mac)
        {
            retryTimer.Stop();
            onDone(mac);
            return;
        }

        if (retriesLeft == 0)
        {
            retryTimer.Stop();
            onDone(std::nullopt);
        }
        else
        {
            --retriesLeft;
            sendRequest(pendingAddress);
            retryTimer.Start(infra::FailureType::intermittentFailure, [this]
                {
                    OnRetry();
                });
        }
    }

    ArpMacResolverLwIp::ArpMacResolverLwIp(uint8_t retries, infra::Duration retryInterval)
        : helper(retries, retryInterval, [this](const IPAddress& address)
              {
                  return Lookup(address);
              },
              [this](const IPAddress& address)
              {
                  SendRequest(address);
              })
    {}

    void ArpMacResolverLwIp::Resolve(const IPAddress& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone)
    {
        const auto* ipv4 = std::get_if<IPv4Address>(&address);
        if (ipv4 == nullptr || !IsOnSubnet(*ipv4))
        {
            onResolveDone(std::nullopt);
            return;
        }
        helper.Resolve(address, onResolveDone);
    }

    bool ArpMacResolverLwIp::IsOnSubnet(const IPv4Address& address) const
    {
        ip4_addr_t target;
        IP4_ADDR(&target, address[0], address[1], address[2], address[3]);
        return ip4_addr_netcmp(&target, netif_ip4_addr(netif_default), netif_ip4_netmask(netif_default));
    }

    std::optional<hal::MacAddress> ArpMacResolverLwIp::Lookup(const IPAddress& address) const
    {
        const auto* ipv4 = std::get_if<IPv4Address>(&address);
        if (ipv4 == nullptr || !IsOnSubnet(*ipv4))
            return std::nullopt;

        ip4_addr_t target;
        IP4_ADDR(&target, (*ipv4)[0], (*ipv4)[1], (*ipv4)[2], (*ipv4)[3]);

        struct eth_addr* ethaddr = nullptr;
        if (const ip4_addr_t* ipaddr = nullptr; etharp_find_addr(netif_default, &target, &ethaddr, &ipaddr) >= 0 && ethaddr != nullptr)
        {
            hal::MacAddress mac;
            std::copy(ethaddr->addr, ethaddr->addr + mac.size(), mac.begin());
            return mac;
        }
        return std::nullopt;
    }

    void ArpMacResolverLwIp::SendRequest(const IPAddress& address) const
    {
        const auto* ipv4 = std::get_if<IPv4Address>(&address);
        if (ipv4 == nullptr || !IsOnSubnet(*ipv4))
            return;
        ip4_addr_t target;
        IP4_ADDR(&target, (*ipv4)[0], (*ipv4)[1], (*ipv4)[2], (*ipv4)[3]);
        etharp_request(netif_default, &target);
    }

    Nd6MacResolverLwIp::Nd6MacResolverLwIp(uint8_t retries, infra::Duration retryInterval)
        : helper(retries, retryInterval, [this](const IPAddress& address)
              {
                  return Lookup(address);
              },
              [this](const IPAddress& address)
              {
                  SendRequest(address);
              })
    {}

    void Nd6MacResolverLwIp::Resolve(const IPAddress& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone)
    {
        const auto* ipv6 = std::get_if<IPv6Address>(&address);
        if (ipv6 == nullptr || !IsOnLink(*ipv6))
        {
            onResolveDone(std::nullopt);
            return;
        }
        helper.Resolve(address, onResolveDone);
    }

    bool Nd6MacResolverLwIp::IsOnLink(const IPv6Address& address) const
    {
        auto target = ToLwipIp6(address);
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; ++i)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(netif_default, i)))
            {
                auto* netifAddr = netif_ip6_addr(netif_default, i);
                if (ip6_addr_netcmp(&target, netifAddr))
                    return true;
            }
        }
        return false;
    }

    std::optional<hal::MacAddress> Nd6MacResolverLwIp::Lookup(const IPAddress& address) const
    {
        const auto* ipv6 = std::get_if<IPv6Address>(&address);
        if (ipv6 == nullptr || !IsOnLink(*ipv6))
            return std::nullopt;
        auto target = ToLwipIp6(*ipv6);

        const u8_t* hwaddrp = nullptr;
        auto* probe = pbuf_alloc_reference(nullptr, 0, PBUF_REF);
        if (probe == nullptr)
            return std::nullopt;

        // nd6_get_next_hop_addr_or_queue() enqueues its own copy of the packet if neighbour
        // discovery needs to be triggered; it does not retain the caller's pbuf. probe can
        // therefore be freed unconditionally after this call.
        auto result = nd6_get_next_hop_addr_or_queue(netif_default, probe, &target, &hwaddrp);
        pbuf_free(probe);

        if (result == ERR_OK && hwaddrp != nullptr)
        {
            hal::MacAddress mac;
            std::copy(hwaddrp, hwaddrp + mac.size(), mac.begin());
            return mac;
        }
        return std::nullopt;
    }

    void Nd6MacResolverLwIp::SendRequest(const IPAddress& /* address */) const
    {
        // ND6 solicitation is triggered implicitly via Lookup (nd6_get_next_hop_addr_or_queue)
    }
}
