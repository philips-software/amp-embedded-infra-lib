#ifndef HAL_MAC_ADDRESS_HPP
#define HAL_MAC_ADDRESS_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include <array>

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

    class AsMacAddressWriterHelper
    {
    public:
        explicit AsMacAddressWriterHelper(hal::MacAddress& macAddress);

        friend TextInputStream& operator>>(TextInputStream& stream, AsMacAddressWriterHelper asMacAddressHelper);
        friend TextInputStream& operator>>(TextInputStream&& stream, AsMacAddressWriterHelper asMacAddressHelper);

    private:
        hal::MacAddress& macAddress;
    };

    AsMacAddressHelper AsMacAddress(hal::MacAddress macAddress);
    AsMacAddressWriterHelper ToMacAddress(hal::MacAddress& macAddress);
}
#endif
