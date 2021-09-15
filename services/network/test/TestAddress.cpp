#include "network/Address.hpp"
#include "gmock/gmock.h"

namespace
{
    class AddressTest
        : public testing::Test
    {};
}

TEST_F(AddressTest, ParseIpv4Address)
{
    services::IPv4Address ipv4Address{ 111, 122, 133, 144 };
    EXPECT_EQ(ipv4Address, *services::ParseIpv4Address("111.122.133.144"));

    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.133.444"));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.133."));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.133.144."));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.3.1A"));
    EXPECT_EQ(infra::none, services::ParseIpv4Address("111.122.3.0144"));
}

TEST_F(AddressTest, ParseFullIpv6Address)
{
    services::IPv6Address ipv6Address{ 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888 };
    EXPECT_EQ(ipv6Address, *services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:8888"));

    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:"));
    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:8888:"));
    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:888G"));
    EXPECT_EQ(infra::none, services::ParseFullIpv6Address("1111:2222:3333:4444:5555:6666:7777:88889"));
}

TEST_F(AddressTest, ParseIpAddress)
{
    services::IPAddress ipv4Address{ services::IPv4Address{ 111, 122, 133, 144 } };
    EXPECT_EQ(ipv4Address, *services::ParseIpAddress("111.122.133.144"));

    services::IPAddress ipv6Address{ services::IPv6Address{ 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888 } };
    EXPECT_EQ(ipv6Address, *services::ParseIpAddress("1111:2222:3333:4444:5555:6666:7777:8888"));

    EXPECT_EQ(infra::none, services::ParseIpAddress("111.122.133."));
    EXPECT_EQ(infra::none, services::ParseIpAddress("1111:2222:3333:4444:5555:6666:7777:"));
}
