#ifndef SERVICES_WI_FI_NETWORK_HPP
#define SERVICES_WI_FI_NETWORK_HPP

#include "hal/interfaces/Ethernet.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Observer.hpp"
#include "services/network/Address.hpp"

namespace services
{
    struct WiFiSecurity
    {
        enum class SecurityMode : int8_t
        {
            unknown = -1,
            open = 0,
            wpa2MixedPsk,
            wpa3Psk
        };

        WiFiSecurity() = default;
        WiFiSecurity(const WiFiSecurity& other, infra::BoundedConstString key);

        bool operator==(const WiFiSecurity& other) const;
        bool operator!=(const WiFiSecurity& other) const;

        static WiFiSecurity Wpa2Security(infra::BoundedConstString key);
        static WiFiSecurity Wpa3Security(infra::BoundedConstString key);

        infra::BoundedConstString ToString() const;

        SecurityMode securityMode = SecurityMode::open;

        static const std::size_t minimumSecurityKeySize = 8;
        static const std::size_t securityKeySize = 64;
        infra::BoundedString::WithStorage<securityKeySize> key;
    };

    const uint8_t MaxSsidSize{ 32 };
    using SsidString = infra::BoundedString::WithStorage<MaxSsidSize>;

    struct IPAddresses
    {
        services::IPAddress address;
        services::IPAddress netmask;
        services::IPAddress gateway;

        bool operator==(const IPAddresses& other) const;
        bool operator!=(const IPAddresses& other) const;
    };

    struct IpConfig
    {
        bool useDhcp;
        IPAddresses addresses;

        bool operator==(const IpConfig& other) const;
        bool operator!=(const IpConfig& other) const;
    };

    enum class WiFiJoiningStatus
    {
        noError,
        unknownError,
        timeout,
        authenticationError,
        networkNotFound
    };

    class WiFiNetwork;

    class WiFiNetworkStationObserver
        : public infra::SingleObserver<WiFiNetworkStationObserver, WiFiNetwork>
    {
    protected:
        using infra::SingleObserver<WiFiNetworkStationObserver, WiFiNetwork>::SingleObserver;

    public:
        virtual void LinkDown() = 0;
        virtual void LinkUp() = 0;
        virtual void IpAddressChanged() = 0;
    };

    class WiFiNetworkAccessPointObserver
        : public infra::SingleObserver<WiFiNetworkAccessPointObserver, WiFiNetwork>
    {
    protected:
        using infra::SingleObserver<WiFiNetworkAccessPointObserver, WiFiNetwork>::SingleObserver;

    public:
        virtual void HasClientAssociations() = 0;
        virtual void HasNoClientAssociations() = 0;
    };

    class WiFiNetworkJoinResultObserver
    {
    protected:
        WiFiNetworkJoinResultObserver() = default;
        WiFiNetworkJoinResultObserver(const WiFiNetworkJoinResultObserver& other) = delete;
        WiFiNetworkJoinResultObserver& operator=(const WiFiNetworkJoinResultObserver& other) = delete;
        ~WiFiNetworkJoinResultObserver() = default;

    public:
        virtual void JoinedNetwork() = 0;
        virtual void JoinNetworkFailed(WiFiJoiningStatus error) = 0;
    };

    class WiFiNetworkScanNetworkForDetailsResultObserver
    {
    protected:
        WiFiNetworkScanNetworkForDetailsResultObserver() = default;
        WiFiNetworkScanNetworkForDetailsResultObserver(const WiFiNetworkScanNetworkForDetailsResultObserver& other) = delete;
        WiFiNetworkScanNetworkForDetailsResultObserver& operator=(const WiFiNetworkScanNetworkForDetailsResultObserver& other) = delete;
        ~WiFiNetworkScanNetworkForDetailsResultObserver() = default;

    public:
        virtual void NetworkDetailsAvailable(const hal::MacAddress& bssid, uint8_t channel, const WiFiSecurity& security) = 0;
        virtual void NetworkDetailsUnavailable() = 0;
    };

    class WiFiNetworkScanNetworksResultObserver
    {
    protected:
        WiFiNetworkScanNetworksResultObserver() = default;
        WiFiNetworkScanNetworksResultObserver(const WiFiNetworkScanNetworksResultObserver& other) = delete;
        WiFiNetworkScanNetworksResultObserver& operator=(const WiFiNetworkScanNetworksResultObserver& other) = delete;
        ~WiFiNetworkScanNetworksResultObserver() = default;

    public:
        struct Network
        {
        public:
            Network(infra::BoundedConstString ssid, int32_t signalStrength, const WiFiSecurity& security);

            infra::BoundedConstString ssid;
            int32_t signalStrength;
            WiFiSecurity security;
        };

        virtual void NetworksFound(infra::MemoryRange<const Network> networks) = 0;
    };

    class WiFiNetwork
        : public infra::Subject<WiFiNetworkStationObserver>
        , public infra::Subject<WiFiNetworkAccessPointObserver>
    {
    public:
        virtual void StartAccessPoint(infra::BoundedConstString ssid, const WiFiSecurity& security, uint8_t channel, services::IPAddresses ipSettings, const infra::Function<void()>& onDone) = 0;
        // JoinNetwork eventually results in either JoinedNetwork or JoinNetworkFailed to be called on its observer.
        // Before receiving those callbacks a different state may not be entered.
        virtual void JoinNetwork(infra::BoundedConstString ssid, const WiFiSecurity& security, const IpConfig& ipConfig, WiFiNetworkJoinResultObserver& joinResultObserver) = 0;
        virtual void JoinNetwork(infra::BoundedConstString ssid, hal::MacAddress bssid, uint8_t channel, const WiFiSecurity& security, const IpConfig& ipConfig, WiFiNetworkJoinResultObserver& joinResultObserver) = 0;
        virtual void Stop() = 0;

        virtual bool GetRssi(int32_t& rssiOut) const = 0;
        virtual hal::MacAddress GetMacAddress() const = 0;
        virtual IpConfig GetIpConfig() const = 0;

        virtual bool HasAssociatedClients() const = 0;

    protected:
        ~WiFiNetwork() = default;
    };

    class WiFiNetworkScanner
    {
    protected:
        WiFiNetworkScanner() = default;
        WiFiNetworkScanner(const WiFiNetworkScanner& other) = delete;
        WiFiNetworkScanner& operator=(const WiFiNetworkScanner& other) = delete;
        ~WiFiNetworkScanner() = default;

    public:
        // ScanForNetworkDetails eventually results in either NetworkDetailsAvailable or NetworkDetailsUnavailable
        virtual void ScanForNetworkDetails(infra::BoundedConstString ssid, WiFiNetworkScanNetworkForDetailsResultObserver& observer) = 0;
        // ScanNetworks eventually results in NetworksFound
        virtual void ScanNetworks(int32_t numOfProbesPerChannel, infra::Optional<infra::Duration> waitTimePerChannelActive, WiFiNetworkScanNetworksResultObserver& observer) = 0;
    };

    class NetworkPingResultsObserver
    {
    protected:
        NetworkPingResultsObserver() = default;
        NetworkPingResultsObserver(const NetworkPingResultsObserver& other) = delete;
        NetworkPingResultsObserver& operator=(const NetworkPingResultsObserver& other) = delete;
        ~NetworkPingResultsObserver() = default;

    public:
        virtual void PingSuccess() = 0;
        virtual void PingFailed() = 0;
    };

    class NetworkPing
    {
    protected:
        NetworkPing() = default;
        NetworkPing(const NetworkPing& other) = delete;
        NetworkPing& operator=(const NetworkPing& other) = delete;
        ~NetworkPing() = default;

    public:
        // PingGateway eventually results in either PingSuccess or PingFailed
        virtual void PingGateway(infra::Duration timeout, NetworkPingResultsObserver& observer) = 0;
    };
}

#endif
