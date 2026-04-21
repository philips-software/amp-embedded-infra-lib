#include "lwip/lwip_cpp/MacResolverLwIp.hpp"
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

        // lwIP has no public API to trigger a standalone ND6 solicitation.
        // We therefore create a zero-length PBUF_REF probe and call
        // nd6_get_next_hop_addr_or_queue(), which drives neighbor discovery.
        // If queued, lwIP clones this volatile probe, so the caller still
        // releases the original pbuf after the lookup call returns.
        std::optional<hal::MacAddress> Nd6Lookup(const ip6_addr_t& target)
        {
            const u8_t* hwaddrp = nullptr;
            auto* probe = pbuf_alloc_reference(nullptr, 0, PBUF_REF);
            if (probe == nullptr)
            {
                return std::nullopt;
            }

            auto result = nd6_get_next_hop_addr_or_queue(netif_default, probe, &target, &hwaddrp);
            pbuf_free(probe);

            if (result >= 0 && hwaddrp != nullptr)
            {
                hal::MacAddress mac;
                std::copy(hwaddrp, hwaddrp + mac.size(), mac.begin());
                return mac;
            }
            return std::nullopt;
        }
    }

    void ArpMacResolverLwIp::SendRequest(IPv4Address address)
    {
        ip4_addr_t target;
        IP4_ADDR(&target, address[0], address[1], address[2], address[3]);
        etharp_request(netif_default, &target);
    }

    std::optional<hal::MacAddress> ArpMacResolverLwIp::Lookup(IPv4Address address)
    {
        ip4_addr_t target;
        IP4_ADDR(&target, address[0], address[1], address[2], address[3]);

        struct eth_addr* ethaddr = nullptr;
        const ip4_addr_t* ipaddr = nullptr;
        if (etharp_find_addr(netif_default, &target, &ethaddr, &ipaddr) >= 0 && ethaddr != nullptr)
        {
            hal::MacAddress mac;
            std::copy(ethaddr->addr, ethaddr->addr + mac.size(), mac.begin());
            return mac;
        }
        return std::nullopt;
    }

    bool ArpMacResolverLwIp::Resolve(IPv4Address address, uint8_t retries, infra::Duration retryInterval, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone)
    {
        if (retryTimer_.Armed())
        {
            return false;
        }

        auto mac = Lookup(address);
        if (mac)
        {
            onDone(mac);
            return true;
        }

        pendingAddress_ = address;
        onDone_ = onDone;
        retriesLeft_ = retries;

        SendRequest(address);

        retryTimer_.Start(retryInterval, [this]
            {
                OnRetry();
            });

        return true;
    }

    void ArpMacResolverLwIp::OnRetry()
    {
        auto mac = Lookup(pendingAddress_);
        if (mac)
        {
            retryTimer_.Cancel();
            onDone_(mac);
            return;
        }

        if (retriesLeft_ == 0)
        {
            retryTimer_.Cancel();
            onDone_(std::nullopt);
        }
        else
        {
            --retriesLeft_;
            SendRequest(pendingAddress_);
        }
    }

    bool Nd6MacResolverLwIp::Resolve(const IPv6Address& address, uint8_t retries, infra::Duration retryInterval, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone)
    {
        if (retryTimer_.Armed())
        {
            return false;
        }

        auto target = ToLwipIp6(address);
        auto mac = Nd6Lookup(target);
        if (mac)
        {
            onDone(mac);
            return true;
        }

        pendingAddress_ = address;
        onDone_ = onDone;
        retriesLeft_ = retries;

        retryTimer_.Start(retryInterval, [this]
            {
                OnRetry();
            });

        return true;
    }

    void Nd6MacResolverLwIp::OnRetry()
    {
        auto mac = Nd6Lookup(ToLwipIp6(pendingAddress_));

        if (mac)
        {
            retryTimer_.Cancel();
            onDone_(mac);
            return;
        }

        if (retriesLeft_ == 0)
        {
            retryTimer_.Cancel();
            onDone_(std::nullopt);
        }
        else
        {
            --retriesLeft_;
        }
    }
}
