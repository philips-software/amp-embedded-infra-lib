#ifndef LWIP_MAC_RESOLVER_LW_IP_HPP
#define LWIP_MAC_RESOLVER_LW_IP_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "services/network/MacResolver.hpp"

namespace services
{
    class MacResolverRetryHelper
    {
    public:
        using LookupFunction = infra::Function<std::optional<hal::MacAddress>(const MacResolver::Address&)>;
        using SendRequestFunction = infra::Function<void(const MacResolver::Address&)>;

        MacResolverRetryHelper(uint8_t retries, infra::Duration retryInterval, LookupFunction lookup, SendRequestFunction sendRequest);

        void Resolve(const MacResolver::Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone);

    private:
        void OnRetry();

    private:
        uint8_t retries;
        infra::Duration retryInterval;
        LookupFunction lookup;
        SendRequestFunction sendRequest;
        MacResolver::Address pendingAddress;
        infra::AutoResetFunction<void(std::optional<hal::MacAddress>)> onDone;
        infra::TimerRepeating retryTimer;
        uint8_t retriesLeft = 0;
    };

    class ArpMacResolverLwIp
        : public MacResolver
    {
    public:
        ArpMacResolverLwIp(uint8_t retries, infra::Duration retryInterval);
        ~ArpMacResolverLwIp() override = default;

        void Resolve(const Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone) override;

    private:
        std::optional<hal::MacAddress> Lookup(const Address& address) const;
        void SendRequest(const Address& address) const;

    private:
        MacResolverRetryHelper helper;
    };

    class Nd6MacResolverLwIp
        : public MacResolver
    {
    public:
        Nd6MacResolverLwIp(uint8_t retries, infra::Duration retryInterval);
        ~Nd6MacResolverLwIp() override = default;

        void Resolve(const Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onResolveDone) override;

    private:
        std::optional<hal::MacAddress> Lookup(const Address& address) const;
        void SendRequest(const Address& address) const;

    private:
        MacResolverRetryHelper helper;
    };
}

#endif
