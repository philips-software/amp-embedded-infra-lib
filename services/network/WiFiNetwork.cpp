#include "infra/util/CompareMembers.hpp"
#include "services/network/WiFiNetwork.hpp"

namespace services
{
    WiFiSecurity::WiFiSecurity(const WiFiSecurity& other, infra::BoundedConstString key)
        : wep(other.wep)
        , wpa(other.wpa)
        , wpa2(other.wpa2)
        , enterprise(other.enterprise)
        , aes(other.aes)
        , tkip(other.tkip)
        , key(key)
    {}

    bool WiFiSecurity::operator==(const WiFiSecurity& other) const
    {
        return infra::Equals()
            (wep, other.wep)
            (wpa, other.wpa)
            (wpa2, other.wpa2)
            (enterprise, other.enterprise)
            (aes, other.aes)
            (tkip, other.tkip)
            (key, other.key);
    }

    bool WiFiSecurity::operator!=(const WiFiSecurity& other) const
    {
        return !(*this == other);
    }

    WiFiSecurity WiFiSecurity::WepSecurity(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.wep = true;
        result.key = key;
        return result;
    }

    WiFiSecurity WiFiSecurity::WpaSecurity(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.wpa = true;
        result.aes = true;
        result.key = key;
        return result;
    }

    WiFiSecurity WiFiSecurity::Wpa2Security(infra::BoundedConstString key)
    {
        WiFiSecurity result;
        result.wpa2 = true;
        result.aes = true;
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
