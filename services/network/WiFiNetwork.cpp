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
        }

        return "unknown";
    }

    bool IPAddresses::operator==(const IPAddresses& other) const
    {
        return infra::Equals()(address, other.address)(netmask, other.netmask)(gateway, other.gateway);
    }

    bool IPAddresses::operator!=(const IPAddresses& other) const
    {
        return !(*this == other);
    }

    bool IpConfig::operator==(const IpConfig& other) const
    {
        return infra::Equals()(useDhcp, other.useDhcp)(addresses, other.addresses);
    }

    bool IpConfig::operator!=(const IpConfig& other) const
    {
        return !(*this == other);
    }

    WiFiNetworkScanNetworksResultObserver::Network::Network(infra::BoundedConstString ssid, int32_t signalStrength, const WiFiSecurity& security)
        : ssid(ssid)
        , signalStrength(signalStrength)
        , security(security)
    {}
}
