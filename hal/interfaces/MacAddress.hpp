#ifndef HAL_MAC_ADDRESS_HPP
#define HAL_MAC_ADDRESS_HPP

#include <array>
#include "infra/stream/OutputStream.hpp"

namespace hal
{
    using MacAddress = std::array<uint8_t, 6>;
}

namespace infra
{
    class AsMacAddressHelper
    {
    public:
        explicit AsMacAddressHelper(hal::MacAddress macAddress);

        friend TextOutputStream& operator<<(TextOutputStream& stream, const AsMacAddressHelper& asMacAddressHelper);
        friend TextOutputStream& operator<<(TextOutputStream&& stream, const AsMacAddressHelper& asMacAddressHelper);

    private:
        hal::MacAddress macAddress;
    };

    AsMacAddressHelper AsMacAddress(hal::MacAddress macAddress);
}
#endif
