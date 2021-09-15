#include "MacAddress.hpp"

namespace infra
{
    AsMacAddressHelper::AsMacAddressHelper(hal::MacAddress macAddress)
        : macAddress(macAddress)
    {}

    AsMacAddressHelper AsMacAddress(hal::MacAddress macAddress)
    {
        return AsMacAddressHelper(macAddress);
    }

    TextOutputStream& operator<<(TextOutputStream& stream, const AsMacAddressHelper& asMacAddressHelper)
    {
        auto& mac = asMacAddressHelper.macAddress;
        const auto w02 = Width(2, '0');
        stream << hex
               << w02 << mac[0] << resetWidth << ':'
               << w02 << mac[1] << resetWidth << ':'
               << w02 << mac[2] << resetWidth << ':'
               << w02 << mac[3] << resetWidth << ':'
               << w02 << mac[4] << resetWidth << ':'
               << w02 << mac[5];
        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream&& stream, const AsMacAddressHelper& asMacAddressHelper)
    {
        return stream << asMacAddressHelper;
    }
}
