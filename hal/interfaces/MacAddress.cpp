#include "MacAddress.hpp"
#include <algorithm>

namespace infra
{
    AsMacAddressHelper::AsMacAddressHelper(hal::MacAddress macAddress)
        : macAddress(macAddress)
    {}

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

    ToMacAddressHelper::ToMacAddressHelper(hal::MacAddress& macAddress)
        : macAddress(macAddress)
    {}

    TextInputStream& operator>>(TextInputStream& stream, ToMacAddressHelper asMacAddressHelper)
    {
        auto& mac = asMacAddressHelper.macAddress;
        const auto w02 = Width(2, '0');
        stream >> hex >> w02 >> mac[0] >> resetWidth >> ":" >> w02 >> mac[1] >> resetWidth >> ":" >> w02 >> mac[2] >> resetWidth >> ":" >> w02 >> mac[3] >> resetWidth >> ":" >> w02 >> mac[4] >> resetWidth >> ":" >> w02 >> mac[5];
        return stream;
    }

    TextInputStream& operator>>(TextInputStream&& stream, ToMacAddressHelper asMacAddressHelper)
    {
        return stream >> asMacAddressHelper;
    }

    ToLittleEndianMacAddressHelper::ToLittleEndianMacAddressHelper(hal::MacAddress& macAddress)
        : macAddress(macAddress)
    {}

    TextInputStream& operator>>(TextInputStream& stream, ToLittleEndianMacAddressHelper asMacAddressHelper)
    {
        auto& mac = asMacAddressHelper.macAddress;
        const auto w02 = Width(2, '0');
        stream >> hex >> w02 >> mac[5] >> resetWidth >> ":" >> w02 >> mac[4] >> resetWidth >> ":" >> w02 >> mac[3] >> resetWidth >> ":" >> w02 >> mac[2] >> resetWidth >> ":" >> w02 >> mac[1] >> resetWidth >> ":" >> w02 >> mac[0];
        return stream;
    }

    TextInputStream& operator>>(TextInputStream&& stream, ToLittleEndianMacAddressHelper asMacAddressHelper)
    {
        return stream >> asMacAddressHelper;
    }

    AsMacAddressHelper AsMacAddress(hal::MacAddress macAddress)
    {
        return AsMacAddressHelper{ macAddress };
    }

    ToMacAddressHelper ToMacAddress(hal::MacAddress& macAddress)
    {
        return ToMacAddressHelper{ macAddress };
    }

    AsMacAddressHelper AsLittleEndianMacAddress(hal::MacAddress macAddress)
    {
        std::reverse(std::begin(macAddress), std::end(macAddress));
        return AsMacAddressHelper{ macAddress };
    }

    ToLittleEndianMacAddressHelper ToLittleEndianMacAddress(hal::MacAddress& macAddress)
    {
        return ToLittleEndianMacAddressHelper{ macAddress };
    }
}
