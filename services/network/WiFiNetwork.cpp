#include "infra/util/CompareMembers.hpp"
#include "services/network/WiFiNetwork.hpp"

namespace services
{
    WiFiSecurity::WiFiSecurity(const WiFiSecurity& other, infra::BoundedConstString key)
        : securityMode(other.securityMode)
        , key(key)
    {}

    bool WiFiSecurity::operator==(const WiFiSecurity& other) const
    {
        return infra::Equals()
            (securityMode, other.securityMode)
            (key, other.key);
    }

    bool WiFiSecurity::operator!=(const WiFiSecurity& other) const
    {
        return !(*this == other);
    }

    WiFiSecurity WiFiSecurity::WepSecurity(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.securityMode = SecurityMode::wepShared;
        result.key = key;
        return result;
    }

    WiFiSecurity WiFiSecurity::WpaSecurity(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.securityMode = SecurityMode::wpaMixedPsk;
        result.key = key;
        return result;
    }

    WiFiSecurity WiFiSecurity::Wpa2Security(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.securityMode = SecurityMode::wpa2MixedPsk;
        result.key = key;
        return result;
    }

    bool IPAddresses::operator==(const IPAddresses& other) const
    {
        return infra::Equals()
            (address, other.address)
            (netmask, other.netmask)
            (gateway, other.gateway);
    }

    bool IPAddresses::operator!=(const IPAddresses& other) const
    {
        return !(*this == other);
    }

    bool IpConfig::operator==(const IpConfig& other) const
    {
        return infra::Equals()
            (useDhcp, other.useDhcp)
            (addresses, other.addresses);
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
