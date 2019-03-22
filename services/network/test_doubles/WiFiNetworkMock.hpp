#ifndef SERVICES_WIFI_NETWORK_MOCK_HPP
#define SERVICES_WIFI_NETWORK_MOCK_HPP

#include "gmock/gmock.h"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "services/network/WiFiNetwork.hpp"

namespace services
{
    //TICS -INT#002: A mock or stub may have public data
    class WiFiNetworkMock
        : public services::WiFiNetwork
    {
    public:
        MOCK_METHOD4(StartAccessPoint, void(infra::BoundedConstString ssid, const services::WiFiSecurity& security, uint8_t channel, services::IPAddresses ipSettings));
        MOCK_METHOD4(JoinNetwork, void(infra::BoundedConstString, const services::WiFiSecurity&, const services::IpConfig&, services::WiFiNetworkJoinResultObserver& joinResultObserver));
        MOCK_METHOD6(JoinNetwork, void(infra::BoundedConstString, hal::MacAddress, uint8_t, const services::WiFiSecurity&, const services::IpConfig&, services::WiFiNetworkJoinResultObserver& joinResultObserver));
        MOCK_METHOD0(Stop, void());

        MOCK_CONST_METHOD1(GetRssi, bool(int32_t&));
        MOCK_CONST_METHOD0(GetMacAddress, hal::MacAddress());
        MOCK_CONST_METHOD0(GetIpConfig, services::IpConfig());

        MOCK_CONST_METHOD0(HasAssociatedClients, bool());
    };

    class WiFiNetworkScannerMock
        : public services::WiFiNetworkScanner
    {
    public:
        MOCK_METHOD2(ScanForNetworkDetails, void(infra::BoundedConstString, services::WiFiNetworkScanNetworkForDetailsResultObserver& observer));
        MOCK_METHOD3(ScanNetworks, void(int32_t numOfProbsPerChannel, infra::Optional<infra::Duration> waitTimePerChannelActive, services::WiFiNetworkScanNetworksResultObserver& observer));
    };

    class WiFiNetworkJoinResultObserverMock
        : public services::WiFiNetworkJoinResultObserver
    {
    public:
        MOCK_METHOD0(JoinedNetwork, void());
        MOCK_METHOD1(JoinNetworkFailed, void(services::WiFiJoiningStatus error));
    };

    class WiFiNetworkScanNetworkForDetailsResultObserverMock
        : public services::WiFiNetworkScanNetworkForDetailsResultObserver
    {
    public:
        MOCK_METHOD3(NetworkDetailsAvailable, void(const hal::MacAddress& bssid, uint8_t channel, const services::WiFiSecurity& security));
        MOCK_METHOD0(NetworkDetailsUnavailable, void());
    };

    class WiFiNetworkScanNetworksResultObserverMock
        : public services::WiFiNetworkScanNetworksResultObserver
    {
    public:
        MOCK_METHOD1(NetworksFound, void(infra::MemoryRange<const Network> networks));
    };

    class NetworkPingMock
        : public services::NetworkPing
    {
    public:
        MOCK_METHOD2(PingGateway, void(infra::Duration timeout, services::NetworkPingResultsObserver& observer));
    };
}

#endif
