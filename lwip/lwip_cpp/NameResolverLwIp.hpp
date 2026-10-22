#ifndef LWIP_NAME_RESOLVER_LW_IP_HPP
#define LWIP_NAME_RESOLVER_LW_IP_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/InterfaceConnector.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/MemoryRange.hpp"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "services/network/Address.hpp"
#include "services/network/NameResolver.hpp"
#include <array>

namespace services
{
    class NameResolverLwIp
        : public NameResolver
        , public infra::InterfaceConnector<NameResolverLwIp>
    {
    public:
        explicit NameResolverLwIp(infra::MemoryRange<const IPAddress> fallbackServers);
        ~NameResolverLwIp();

        void Lookup(NameResolverResult& result) override;
        void CancelLookup(NameResolverResult& result) override;

    private:
        struct Request
        {
            bool pending = false;
        };

        void ScheduleNext();
        void StartNext();
        void Resolve(const NameResolverResult& result);
        void Submit(const char* name, IPVersions versions);
        Request& AcquireRequest();
        void Await(Request& request);
        void CompleteActive(const ip_addr_t* address);
        void Deliver(NameResolverResult& result, const ip_addr_t* address) const;
        void OnFound(Request& request, const ip_addr_t* address);
        void OnTimeout();
        void PrepareServers() const;
        void InstallFallbackServers() const;
        void DropFallbackLeftover() const;
        ip_addr_t FallbackServer(std::size_t index) const;
        void ClearServers() const;
        static bool IsFree(const Request& request);
        static void StaticFound(const char* name, const ip_addr_t* address, void* argument);
        static void StaticExtCallback(netif* network, netif_nsc_reason_t reason, const netif_ext_callback_args_t* args);

    private:
        infra::MemoryRange<const IPAddress> fallbackServers;
        infra::IntrusiveList<NameResolverResult> waiting;
        NameResolverResult* active = nullptr;
        infra::TimerSingleShot timeoutTimer;
        netif_ext_callback_t extCallback;
        std::array<Request, DNS_TABLE_SIZE + 1> requests;
        Request* awaited = nullptr;
    };
}

#endif
