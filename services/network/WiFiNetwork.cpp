#include "services/network/WiFiNetwork.hpp"
#include "infra/util/CompareMembers.hpp"

namespace services
{
    WiFiSecurity::WiFiSecurity(const WiFiSecurity& other, infra::BoundedConstString key)
        : securityMode(other.securityMode)
        , key(key)
    {}

    bool WiFiSecurity::operator==(const WiFiSecurity& other) const
    {
        return infra::Equals()(securityMode, other.securityMode)(key, other.key);
    }

    bool WiFiSecurity::operator!=(const WiFiSecurity& other) const
    {
        return !(*this == other);
    }

    WiFiSecurity WiFiSecurity::Wpa2Security(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.securityMode = SecurityMode::wpa2MixedPsk;
        result.key = key;
        return result;
    }

    WiFiSecurity WiFiSecurity::Wpa3Security(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.securityMode = SecurityMode::wpa3Psk;
        result.key = key;
        return result;
    }

    infra::BoundedConstString WiFiSecurity::ToString() const
    {
        switch (securityMode)
        {
            case SecurityMode::open:
                return "open";
            case SecurityMode::wpa2MixedPsk:
                return "wpa-2";
            case SecurityMode::wpa3Psk:
                return "wpa-3";
            default:
                return "unknown";
        }
    }

    WiFiNetworkScanNetworksResultObserver::Network::Network(infra::BoundedConstString ssid, int32_t signalStrength, const WiFiSecurity& security)
        : ssid(ssid)
        , signalStrength(signalStrength)
        , security(security)
    {}
}
