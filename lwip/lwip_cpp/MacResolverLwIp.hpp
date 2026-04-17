#ifndef LWIP_MAC_RESOLVER_LW_IP_HPP
#define LWIP_MAC_RESOLVER_LW_IP_HPP

#include "infra/timer/Timer.hpp"
#include "services/network/MacResolver.hpp"

namespace services
{
    class ArpMacResolverLwIp
        : public ArpMacResolver
    {
    public:
        void SendRequest(IPv4Address address) override;
        std::optional<hal::MacAddress> Lookup(IPv4Address address) override;
        void Resolve(IPv4Address address, uint8_t retries, infra::Duration retryInterval, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone) override;

    private:
        void OnRetry();

        IPv4Address pendingAddress_{};
        infra::Function<void(std::optional<hal::MacAddress>)> onDone_;
        infra::TimerRepeating retryTimer_;
        uint8_t retriesLeft_ = 0;
    };

    class Nd6MacResolverLwIp
        : public Nd6MacResolver
    {
    public:
        void Resolve(const IPv6Address& address, uint8_t retries, infra::Duration retryInterval, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone) override;

    private:
        void OnRetry();

        IPv6Address pendingAddress_{};
        infra::Function<void(std::optional<hal::MacAddress>)> onDone_;
        infra::TimerRepeating retryTimer_;
        uint8_t retriesLeft_ = 0;
    };
}

#endif
