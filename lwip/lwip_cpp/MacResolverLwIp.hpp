#ifndef LWIP_MAC_RESOLVER_LW_IP_HPP
#define LWIP_MAC_RESOLVER_LW_IP_HPP

#include "infra/timer/Retry.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "services/network/MacResolver.hpp"

namespace services
{
    class MacResolverRetryHelper
    {
    public:
        using LookupFunction = infra::Function<std::optional<hal::MacAddress>(const IPAddress&)>;
        using SendRequestFunction = infra::Function<void(const IPAddress&)>;

        MacResolverRetryHelper(uint8_t retries, infra::Duration retryInterval, LookupFunction lookup, SendRequestFunction sendRequest);

        [[nodiscard]] bool Resolve(const IPAddress& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone);

    private:
        void OnRetry();

    private:
        uint8_t retries;
        LookupFunction lookup;
        SendRequestFunction sendRequest;
        IPAddress pendingAddress;
        infra::AutoResetFunction<void(std::optional<hal::MacAddress>)> onDone;
        infra::RetryFixedInterval retryTimer;
        uint8_t retriesLeft = 0;
    };

    class ArpMacResolverLwIp
        : public MacResolver
    {
    public:
        ArpMacResolverLwIp(uint8_t retries, infra::Duration retryInterval);

        [[nodiscard]] bool Resolve(const IPAddress& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone) override;

    private:
        bool IsOnSubnet(const IPv4Address& address) const;
        std::optional<hal::MacAddress> Lookup(const IPAddress& address) const;
        void SendRequest(const IPAddress& address) const;

    private:
        MacResolverRetryHelper helper;
    };

    class Nd6MacResolverLwIp
        : public MacResolver
    {
    public:
        Nd6MacResolverLwIp(uint8_t retries, infra::Duration retryInterval);

        [[nodiscard]] bool Resolve(const IPAddress& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone) override;

    private:
        bool IsOnLink(const IPv6Address& address) const;
        std::optional<hal::MacAddress> Lookup(const IPAddress& address) const;
        void SendRequest(const IPAddress& address) const;

    private:
        MacResolverRetryHelper helper;
    };
}

#endif
