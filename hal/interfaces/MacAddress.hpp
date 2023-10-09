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

    class ToMacAddressHelper
    {
    public:
        explicit ToMacAddressHelper(hal::MacAddress& macAddress);

        friend TextInputStream& operator>>(TextInputStream& stream, ToMacAddressHelper asMacAddressHelper);
        friend TextInputStream& operator>>(TextInputStream&& stream, ToMacAddressHelper asMacAddressHelper);

    protected:
        hal::MacAddress& macAddress;
    };

    class ToLittleEndianMacAddressHelper
    {
    public:
        explicit ToLittleEndianMacAddressHelper(hal::MacAddress& macAddress);

        friend TextInputStream& operator>>(TextInputStream& stream, ToLittleEndianMacAddressHelper asMacAddressHelper);
        friend TextInputStream& operator>>(TextInputStream&& stream, ToLittleEndianMacAddressHelper asMacAddressHelper);

    protected:
        hal::MacAddress& macAddress;
    };

    AsMacAddressHelper AsMacAddress(hal::MacAddress macAddress);
    ToMacAddressHelper ToMacAddress(hal::MacAddress& macAddress);

    AsMacAddressHelper AsLittleEndianMacAddress(hal::MacAddress macAddress);
    ToLittleEndianMacAddressHelper ToLittleEndianMacAddress(hal::MacAddress& macAddress);
}
#endif
