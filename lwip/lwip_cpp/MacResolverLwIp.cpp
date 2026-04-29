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
                PP_HTONL(address[1] | (static_cast<uint32_t>(address[0]) << 16)),
                PP_HTONL(address[3] | (static_cast<uint32_t>(address[2]) << 16)),
                PP_HTONL(address[5] | (static_cast<uint32_t>(address[4]) << 16)),
                PP_HTONL(address[7] | (static_cast<uint32_t>(address[6]) << 16)));
            ip6_addr_set_zone(&target, netif_default->ip6_addr->u_addr.ip6.zone);
            return target;
        }
    }

    MacResolverRetryHelper::MacResolverRetryHelper(uint8_t retries, infra::Duration retryInterval, LookupFunction lookup, SendRequestFunction sendRequest)
        : retries(retries)
        , retryInterval(retryInterval)
        , lookup(lookup)
        , sendRequest(sendRequest)
    {}

    void MacResolverRetryHelper::Resolve(const MacResolver::Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone)
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
        retryTimer.Start(retryInterval, [this] { OnRetry(); });
    }

    void MacResolverRetryHelper::OnRetry()
    {
        if (auto mac = lookup(pendingAddress); mac)
        {
            retryTimer.Cancel();
            onDone(mac);
            return;
        }

        if (retriesLeft == 0)
        {
            retryTimer.Cancel();
            onDone(std::nullopt);
        }
        else
        {
            --retriesLeft;
            sendRequest(pendingAddress);
        }
    }

    ArpMacResolverLwIp::ArpMacResolverLwIp(uint8_t retries, infra::Duration retryInterval)
        : helper(retries, retryInterval,
            [this](const Address& address) { return Lookup(address); },
            [this](const Address& address) { SendRequest(address); })
    {}

    void ArpMacResolverLwIp::Resolve(const Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone)
    {
        helper.Resolve(address, onResolveDone);
    }

    std::optional<hal::MacAddress> ArpMacResolverLwIp::Lookup(const Address& address) const
    {
        ip4_addr_t target;
        auto& ipv4 = std::get<IPv4Address>(address);
        IP4_ADDR(&target, ipv4[0], ipv4[1], ipv4[2], ipv4[3]);

        struct eth_addr* ethaddr = nullptr;
        if (const ip4_addr_t* ipaddr = nullptr; etharp_find_addr(netif_default, &target, &ethaddr, &ipaddr) >= 0 && ethaddr != nullptr)
        {
            hal::MacAddress mac;
            std::copy(ethaddr->addr, ethaddr->addr + mac.size(), mac.begin());
            return mac;
        }
        return std::nullopt;
    }

    void ArpMacResolverLwIp::SendRequest(const Address& address) const
    {
        ip4_addr_t target;
        auto& ipv4 = std::get<IPv4Address>(address);
        IP4_ADDR(&target, ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
        etharp_request(netif_default, &target);
    }

    Nd6MacResolverLwIp::Nd6MacResolverLwIp(uint8_t retries, infra::Duration retryInterval)
        : helper(retries, retryInterval,
            [this](const Address& address) { return Lookup(address); },
            [this](const Address& address) { SendRequest(address); })
    {}

    void Nd6MacResolverLwIp::Resolve(const Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone)
    {
        helper.Resolve(address, onResolveDone);
    }

    std::optional<hal::MacAddress> Nd6MacResolverLwIp::Lookup(const Address& address) const
    {
        auto& ipv6 = std::get<IPv6Address>(address);
        auto target = ToLwipIp6(ipv6);

        // Check if target is on-link by matching against netif prefixes
        bool onLink = false;
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; ++i)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(netif_default, i)))
            {
                auto* netifAddr = netif_ip6_addr(netif_default, i);
                if (ip6_addr_netcmp(&target, netifAddr))
                {
                    onLink = true;
                    break;
                }
            }
        }

        if (!onLink)
            return std::nullopt;

        const u8_t* hwaddrp = nullptr;
        auto* probe = pbuf_alloc_reference(nullptr, 0, PBUF_REF);
        if (probe == nullptr)
            return std::nullopt;

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

    void Nd6MacResolverLwIp::SendRequest(const Address& /* address */) const
    {
        // ND6 solicitation is triggered implicitly via Lookup (nd6_get_next_hop_addr_or_queue)
    }
}
