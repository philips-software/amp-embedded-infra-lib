#include "services/network_instantiations/SocketAddress.hpp"
#include "gmock/gmock.h"
#include <cstring>

namespace
{
    using services::detail::AddressFamily;
    using services::detail::AnyAddressSocket;
    using services::detail::FillSocketAddress;
}

class SocketAddressTest
    : public testing::Test
{
public:
    sockaddr_storage storage{};
};

TEST_F(SocketAddressTest, AddressFamilyFromIpVersions)
{
    EXPECT_THAT(AddressFamily(services::IPVersions::ipv4), testing::Eq(AF_INET));
    EXPECT_THAT(AddressFamily(services::IPVersions::ipv6), testing::Eq(AF_INET6));
    EXPECT_THAT(AddressFamily(services::IPVersions::both), testing::Eq(AF_INET));
}

TEST_F(SocketAddressTest, AddressFamilyFromUdpSocket)
{
    EXPECT_THAT(AddressFamily(services::UdpSocket{ services::Udpv4Socket{ services::IPv4Address{}, 0 } }), testing::Eq(AF_INET));
    EXPECT_THAT(AddressFamily(services::UdpSocket{ services::Udpv6Socket{ services::IPv6Address{}, 0 } }), testing::Eq(AF_INET6));
}

TEST_F(SocketAddressTest, AddressFamilyFromIpAddress)
{
    EXPECT_THAT(AddressFamily(services::IPAddress{ services::IPv4Address{} }), testing::Eq(AF_INET));
    EXPECT_THAT(AddressFamily(services::IPAddress{ services::IPv6Address{} }), testing::Eq(AF_INET6));
}

TEST_F(SocketAddressTest, AnyAddressSocketForIpv4)
{
    EXPECT_THAT(AnyAddressSocket(AF_INET, 1234), testing::Eq(services::UdpSocket{ services::Udpv4Socket{ services::IPv4Address{}, 1234 } }));
}

TEST_F(SocketAddressTest, AnyAddressSocketForIpv6)
{
    EXPECT_THAT(AnyAddressSocket(AF_INET6, 5678), testing::Eq(services::UdpSocket{ services::Udpv6Socket{ services::IPv6Address{}, 5678 } }));
}

TEST_F(SocketAddressTest, FillSocketAddressForIpv4)
{
    services::IPv4Address ipv4{ 192, 168, 1, 5 };
    auto size = FillSocketAddress(services::UdpSocket{ services::Udpv4Socket{ ipv4, 8080 } }, storage);

    auto& address = reinterpret_cast<sockaddr_in&>(storage);
    EXPECT_THAT(size, testing::Eq(sizeof(sockaddr_in)));
    EXPECT_THAT(address.sin_family, testing::Eq(AF_INET));
    EXPECT_THAT(address.sin_port, testing::Eq(htons(8080)));
    EXPECT_THAT(address.sin_addr.s_addr, testing::Eq(htonl(services::ConvertToUint32(ipv4))));
}

TEST_F(SocketAddressTest, FillSocketAddressForIpv6)
{
    services::IPv6Address ipv6{ 0xfe80, 0, 0, 0, 0x0202, 0xb3ff, 0xfe1e, 0x8329 };
    auto size = FillSocketAddress(services::UdpSocket{ services::Udpv6Socket{ ipv6, 9090 } }, storage);

    auto& address = reinterpret_cast<sockaddr_in6&>(storage);
    auto networkOrder = services::ToNetworkOrder(ipv6);

    EXPECT_THAT(size, testing::Eq(sizeof(sockaddr_in6)));
    EXPECT_THAT(address.sin6_family, testing::Eq(AF_INET6));
    EXPECT_THAT(address.sin6_port, testing::Eq(htons(9090)));
    EXPECT_THAT(std::memcmp(&address.sin6_addr, networkOrder.data(), sizeof(address.sin6_addr)), testing::Eq(0));
}
