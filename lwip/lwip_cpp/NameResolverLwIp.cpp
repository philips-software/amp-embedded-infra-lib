#include "lwip/lwip_cpp/NameResolverLwIp.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/lwip_cpp/IpAddressLwIp.hpp"
#include "lwip/netif.h"
#include <algorithm>
#include <chrono>
#include <utility>

#if !LWIP_DNS
#error "NameResolverLwIp requires LWIP_DNS"
#endif

namespace services
{
    namespace
    {
        constexpr infra::Duration validity = std::chrono::minutes(5);
        constexpr infra::Duration requestTimeout = std::chrono::seconds(30);

        u8_t ToAddressType(IPVersions versions)
        {
            switch (versions)
            {
                case IPVersions::ipv4:
                    return LWIP_DNS_ADDRTYPE_IPV4;
                case IPVersions::ipv6:
                    return LWIP_DNS_ADDRTYPE_IPV6;
                default:
                    return LWIP_DNS_ADDRTYPE_IPV4_IPV6;
            }
        }

        std::array<char, DNS_MAX_NAME_LENGTH> NullTerminated(infra::BoundedConstString hostname)
        {
            std::array<char, DNS_MAX_NAME_LENGTH> name;
            std::copy(hostname.begin(), hostname.end(), name.begin());
            name[hostname.size()] = '\0';
            return name;
        }

        bool InvalidatesServers(const netif* network, netif_nsc_reason_t reason)
        {
            return (reason & LWIP_NSC_LINK_CHANGED) != 0 || ((reason & LWIP_NSC_IPV4_ADDRESS_CHANGED) != 0 && ip4_addr_isany(netif_ip4_addr(network)));
        }
    }

    NameResolverLwIp::NameResolverLwIp(infra::MemoryRange<const IPAddress> fallbackServers)
        : fallbackServers(fallbackServers)
    {
        really_assert(!fallbackServers.empty() && fallbackServers.size() <= DNS_MAX_SERVERS);

        netif_add_ext_callback(&extCallback, &StaticExtCallback);
    }

    NameResolverLwIp::~NameResolverLwIp()
    {
        netif_remove_ext_callback(&extCallback);
    }

    void NameResolverLwIp::Lookup(NameResolverResult& result)
    {
        really_assert(!waiting.has_element(result));

        waiting.push_back(result);
        ScheduleNext();
    }

    void NameResolverLwIp::CancelLookup(NameResolverResult& result)
    {
        if (&result == active)
            active = nullptr;
        else
        {
            really_assert(waiting.has_element(result));
            waiting.erase(result);
        }
    }

    void NameResolverLwIp::ScheduleNext()
    {
        if (awaited == nullptr)
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    StartNext();
                });
    }

    void NameResolverLwIp::StartNext()
    {
        if (awaited != nullptr || waiting.empty())
            return;

        active = &waiting.front();
        waiting.pop_front();

        PrepareServers();
        Resolve(*active);
    }

    void NameResolverLwIp::Resolve(const NameResolverResult& result)
    {
        infra::BoundedConstString hostname = result.Hostname();

        if (hostname.size() >= DNS_MAX_NAME_LENGTH)
            CompleteActive(nullptr);
        else
            Submit(NullTerminated(hostname).data(), result.Versions());
    }

    void NameResolverLwIp::Submit(const char* name, IPVersions versions)
    {
        Request& request = AcquireRequest();
        ip_addr_t address;
        err_t error = dns_gethostbyname_addrtype(name, &address, &StaticFound, &request, ToAddressType(versions));

        if (error == ERR_INPROGRESS)
            Await(request);
        else if (error == ERR_OK)
            CompleteActive(&address);
        else
            CompleteActive(nullptr);
    }

    NameResolverLwIp::Request& NameResolverLwIp::AcquireRequest()
    {
        auto slot = std::find_if(requests.begin(), requests.end(), &IsFree);
        really_assert(slot != requests.end());

        return *slot;
    }

    void NameResolverLwIp::Await(Request& request)
    {
        request.pending = true;
        awaited = &request;
        timeoutTimer.Start(requestTimeout, [this]()
            {
                OnTimeout();
            });
    }

    void NameResolverLwIp::CompleteActive(const ip_addr_t* address)
    {
        if (auto result = std::exchange(active, nullptr); result != nullptr)
            Deliver(*result, address);

        ScheduleNext();
    }

    void NameResolverLwIp::Deliver(NameResolverResult& result, const ip_addr_t* address) const
    {
        if (address != nullptr)
            result.NameLookupDone(IpAddressLwIp::Convert(*address), infra::Now() + validity);
        else
            result.NameLookupFailed();
    }

    void NameResolverLwIp::OnFound(Request& request, const ip_addr_t* address)
    {
        request.pending = false;

        if (&request != awaited)
            return;

        awaited = nullptr;
        timeoutTimer.Cancel();
        CompleteActive(address);
    }

    void NameResolverLwIp::OnTimeout()
    {
        awaited = nullptr;
        CompleteActive(nullptr);
    }

    void NameResolverLwIp::PrepareServers() const
    {
        if (ip_addr_isany(dns_getserver(0)))
            InstallFallbackServers();
        else
            DropFallbackLeftover();
    }

    void NameResolverLwIp::InstallFallbackServers() const
    {
        for (u8_t index = 0; index != DNS_MAX_SERVERS; ++index)
        {
            ip_addr_t server = FallbackServer(index);
            dns_setserver(index, &server);
        }
    }

    void NameResolverLwIp::DropFallbackLeftover() const
    {
        ip_addr_t firstFallback = FallbackServer(0);
        ip_addr_t secondFallback = FallbackServer(1);

        if (!ip_addr_cmp(dns_getserver(0), &firstFallback) && ip_addr_cmp(dns_getserver(1), &secondFallback))
            dns_setserver(1, nullptr);
    }

    ip_addr_t NameResolverLwIp::FallbackServer(std::size_t index) const
    {
        if (index < fallbackServers.size())
            return IpAddressLwIp::ToIpAddr(fallbackServers[index]);
        else
            return ip_addr_t(*IP_ADDR_ANY);
    }

    void NameResolverLwIp::ClearServers() const
    {
        for (u8_t index = 0; index != DNS_MAX_SERVERS; ++index)
            dns_setserver(index, nullptr);
    }

    bool NameResolverLwIp::IsFree(const Request& request)
    {
        return !request.pending;
    }

    void NameResolverLwIp::StaticFound(const char*, const ip_addr_t* address, void* argument)
    {
        Instance().OnFound(*static_cast<Request*>(argument), address);
    }

    void NameResolverLwIp::StaticExtCallback(netif* network, netif_nsc_reason_t reason, const netif_ext_callback_args_t*)
    {
        if (InvalidatesServers(network, reason))
            Instance().ClearServers();
    }
}
