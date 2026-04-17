#ifndef SERVICES_MAC_RESOLVER_HPP
#define SERVICES_MAC_RESOLVER_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/Function.hpp"
#include "services/network/Address.hpp"
#include <optional>

namespace services
{
    class ArpMacResolver
    {
    protected:
        ArpMacResolver() = default;
        ArpMacResolver(const ArpMacResolver& other) = delete;
        ArpMacResolver& operator=(const ArpMacResolver& other) = delete;
        ~ArpMacResolver() = default;

    public:
        virtual void SendRequest(IPv4Address address) = 0;
        virtual std::optional<hal::MacAddress> Lookup(IPv4Address address) = 0;
        virtual void Resolve(IPv4Address address, uint8_t retries, infra::Duration retryInterval, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone) = 0;
    };

    class Nd6MacResolver
    {
    protected:
        Nd6MacResolver() = default;
        Nd6MacResolver(const Nd6MacResolver& other) = delete;
        Nd6MacResolver& operator=(const Nd6MacResolver& other) = delete;
        ~Nd6MacResolver() = default;

    public:
        virtual void Resolve(const IPv6Address& address, uint8_t retries, infra::Duration retryInterval, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone) = 0;
    };
}

#endif
