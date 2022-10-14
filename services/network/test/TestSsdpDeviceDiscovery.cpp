#include "gmock/gmock.h"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/SsdpDeviceDiscovery.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "services/network/test_doubles/MulticastMock.hpp"

namespace
{
    const uint16_t ssdpPort = 1900;
    const services::IPv4Address ssdpMulticastAddressIpv4 = { 239, 255, 255, 250 };
    const services::IPv6Address ssdpMulticastAddressIpv6 = { 0xff02, 0, 0, 0, 0, 0, 0, 0x0c };
}

class SsdpDeviceDiscoveryTest
    : public testing::Test
{
public:
    ~SsdpDeviceDiscoveryTest()
    {
        ExpectLeaveMulticastIpv4();
        ExpectLeaveMulticastIpv6();
    }

    void ExpectListenBoth()
    {
        EXPECT_CALL(factory, Listen(testing::_, ssdpPort, services::IPVersions::both)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, uint16_t port, services::IPVersions versions)
            {
            auto ptr = datagramExchange.Emplace();
            observer.Attach(*ptr);

            return ptr; }));

        EXPECT_CALL(factory, Listen(testing::_, services::IPVersions::both)).WillOnce(testing::Invoke([this](services::DatagramExchangeObserver& observer, services::IPVersions versions)
            {
            auto ptr = datagramExchangeActiveDiscovery.Emplace();
            observer.Attach(*ptr);

            return ptr; }));
    }

    void ExpectJoinMulticastIpv4()
    {
        EXPECT_CALL(multicast, JoinMulticastGroup(testing::_, ssdpMulticastAddressIpv4));
    }

    void ExpectJoinMulticastIpv6()
    {
        EXPECT_CALL(multicast, JoinMulticastGroup(testing::_, ssdpMulticastAddressIpv6));
    }

    void ExpectLeaveMulticastIpv4()
    {
        EXPECT_CALL(multicast, LeaveMulticastGroup(testing::_, ssdpMulticastAddressIpv4));
    }

    void ExpectLeaveMulticastIpv6()
    {
        EXPECT_CALL(multicast, LeaveMulticastGroup(testing::_, ssdpMulticastAddressIpv6));
    }

    void ExpectActiveQueryStarted(services::IPVersions ipVersion)
    {
        EXPECT_CALL(*datagramExchangeActiveDiscovery, RequestSendStream(testing::_, testing::_)).WillOnce([ipVersion](std::size_t sendSize, services::UdpSocket remote)
            {
                ASSERT_EQ(sendSize, 200);

                if (ipVersion == services::IPVersions::ipv6)
                    ASSERT_TRUE(remote.Is<services::Udpv6Socket>());
                else
                    ASSERT_TRUE(remote.Is<services::Udpv4Socket>()); });
    }

    std::vector<uint8_t> DiscoveryRequestIPv4(std::string searchTarget, std::string maxWaitResponseTime)
    {
        return infra::ConstructBin()("M-SEARCH * HTTP/1.1\r\n")("HOST: 239.255.255.250:1900\r\n")("MAN: \"ssdp:discover\"\r\n")("ST: ")(searchTarget)("\r\n")("MX: ")(maxWaitResponseTime)("\r\n")
            .Vector();
    }

    std::vector<uint8_t> DiscoveryRequestIPv6(std::string searchTarget, std::string maxWaitResponseTime)
    {
        return infra::ConstructBin()("M-SEARCH * HTTP/1.1\r\n")("HOST: [ff02:0:0:0:0:0:0:c]:1900\r\n")("MAN: \"ssdp:discover\"\r\n")("ST: ")(searchTarget)("\r\n")("MX: ")(maxWaitResponseTime)("\r\n")
            .Vector();
    }

    void SendStreamAvailableAndExpectRequest(const std::vector<uint8_t>& data)
    {
        infra::StdVectorOutputStreamWriter::WithStorage response;
        datagramExchangeActiveDiscovery->GetObserver().SendStreamAvailable(infra::UnOwnedSharedPtr(response));
        EXPECT_EQ(data, response.Storage());
    }

public:
    testing::StrictMock<services::DatagramFactoryMock> factory;
    testing::StrictMock<services::MulticastMock> multicast;
    infra::SharedOptional<testing::StrictMock<services::DatagramExchangeMock>> datagramExchange;
    infra::SharedOptional<testing::StrictMock<services::DatagramExchangeMock>> datagramExchangeActiveDiscovery;
    infra::Execute execute{ [this]
        {
            ExpectListenBoth();
            ExpectJoinMulticastIpv4();
            ExpectJoinMulticastIpv6();
        } };
    services::SsdpDeviceDiscovery ssdpDeviceDiscovery{ factory, multicast };
};

TEST_F(SsdpDeviceDiscoveryTest, ssdp_discover_on_ipv4_starts_active_query)
{
    infra::BoundedConstString::WithStorage<12> searchTarget("searchTarget");

    ExpectActiveQueryStarted(services::IPVersions::ipv4);
    ssdpDeviceDiscovery.Discover(searchTarget, 3, services::IPVersions::ipv4);

    auto request = DiscoveryRequestIPv4(infra::AsStdString(searchTarget), "3");
    SendStreamAvailableAndExpectRequest(request);
}

TEST_F(SsdpDeviceDiscoveryTest, ssdp_discover_on_ipv6_starts_active_query)
{
    infra::BoundedConstString::WithStorage<12> searchTarget("searchTarget");

    ExpectActiveQueryStarted(services::IPVersions::ipv6);
    ssdpDeviceDiscovery.Discover(searchTarget, 3, services::IPVersions::ipv6);

    auto request = DiscoveryRequestIPv6(infra::AsStdString(searchTarget), "3");
    SendStreamAvailableAndExpectRequest(request);
}
