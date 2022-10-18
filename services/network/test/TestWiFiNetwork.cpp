#include "gmock/gmock.h"
#include "services/network/WiFiNetwork.hpp"

TEST(WiFiNetworkTest, WiFiSecurity_prints_correct_security_type)
{
    services::WiFiSecurity security;
    security.securityMode = services::WiFiSecurity::SecurityMode::unknown;

    EXPECT_EQ("unknown", security.ToString());
    EXPECT_EQ("open", services::WiFiSecurity{}.ToString());
    EXPECT_EQ("wpa-2", services::WiFiSecurity::Wpa2Security("").ToString());
    EXPECT_EQ("wpa-3", services::WiFiSecurity::Wpa3Security("").ToString());
}

TEST(WiFiNetworkTest, WiFiSecurity_is_comparable)
{
    EXPECT_TRUE(services::WiFiSecurity::Wpa2Security("12345678") == services::WiFiSecurity::Wpa2Security("12345678"));
    EXPECT_FALSE(services::WiFiSecurity::Wpa2Security("12345678") != services::WiFiSecurity::Wpa2Security("12345678"));

    EXPECT_FALSE(services::WiFiSecurity::Wpa2Security("") == services::WiFiSecurity::Wpa2Security("12345678"));
    EXPECT_TRUE(services::WiFiSecurity::Wpa2Security("") != services::WiFiSecurity::Wpa2Security("12345678"));

    EXPECT_FALSE(services::WiFiSecurity::Wpa2Security("12345678") == services::WiFiSecurity::Wpa3Security("12345678"));
    EXPECT_TRUE(services::WiFiSecurity::Wpa2Security("12345678") != services::WiFiSecurity::Wpa3Security("12345678"));
}

TEST(WiFiNetworkTest, WiFiSecurity_is_updated_with_new_key)
{
    auto security = services::WiFiSecurity::Wpa2Security("12345678");
    EXPECT_EQ("87654321", services::WiFiSecurity(security, "87654321").key);
}
