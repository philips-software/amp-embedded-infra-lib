#ifndef SERVICES_MAC_RESOLVER_HPP
#define SERVICES_MAC_RESOLVER_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/Function.hpp"
#include "services/network/Address.hpp"
#include <optional>
#include <variant>

namespace services
{
    class MacResolver
    {
    protected:
        MacResolver() = default;
        MacResolver(const MacResolver& other) = delete;
        MacResolver& operator=(const MacResolver& other) = delete;

    public:
        virtual ~MacResolver() = default;

        using Address = std::variant<IPv4Address, IPv6Address>;
        virtual void Resolve(const Address& address, const infra::Function<void(std::optional<hal::MacAddress>)>& onDone) = 0;
    };
}

#endif
