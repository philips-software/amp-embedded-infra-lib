#include "gmock/gmock.h"
#include "services/network/Address.hpp"

TEST(AddressTest, ParseIpv4Address)
{
    services::IPv4Address ipv4Address{ 111, 122, 133, 144 };
    EXPECT_EQ(ipv4Address, *services::ParseIpv4Address("111.122.133.144"));

    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.133.444"));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.133."));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.133.144."));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.3.1A"));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.3.0144"));
}

TEST(AddressTest, ParseFullIpv6Address)
{
    services::IPv6Address ipv6Address{ 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888 };
    EXPECT_EQ(ipv6Address, *services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:8888"));

    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:"));
    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:8888:"));
    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:888G"));
    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:88889"));
}

TEST(AddressTest, ParseIpAddress)
{
    services::IPAddress ipv4Address{ services::IPv4Address{ 111, 122, 133, 144 } };
    EXPECT_EQ(ipv4Address, *services::ParseIpAddress("111.122.133.144"));

    services::IPAddress ipv6Address{ services::IPv6Address{ 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888 } };
    EXPECT_EQ(ipv6Address, *services::ParseIpAddress("1111:2222:3333:4444:5555:6666:7777:8888"));

    EXPECT_EQ(infra::none, services::ParseIpAddress("111.122.133."));
    EXPECT_EQ(infra::none, services::ParseIpAddress("1111:2222:3333:4444:5555:6666:7777:"));
}

TEST(AddressTest, ConvertIPv4AddressToUint32)
{
    services::IPv4Address ipv4Address{ 0xAA, 0xBB, 0xCC, 0xDD };
    EXPECT_EQ(0xAABBCCDD, services::ConvertToUint32(ipv4Address));
}

TEST(AddressTest, ConvertUint32ToIPv4Address)
{
    services::IPv4Address ipv4Address{ 0xAA, 0xBB, 0xCC, 0xDD };
    EXPECT_EQ(ipv4Address, services::ConvertFromUint32(0xAABBCCDD));
}

TEST(AddressTest, CompareIPv4InterfaceAddresses)
{
    EXPECT_EQ(services::IPv4InterfaceAddresses{}, services::IPv4InterfaceAddresses{});
    EXPECT_NE(services::IPv4InterfaceAddresses{}, (services::IPv4InterfaceAddresses{ services::IPv4Address{ 127, 0, 0, 1 } }));
}

TEST(AddressTest, CompareIpv4Config)
{
    EXPECT_EQ(services::Ipv4Config{}, services::Ipv4Config{});
    EXPECT_NE(services::Ipv4Config{ false }, services::Ipv4Config{ true });
}
