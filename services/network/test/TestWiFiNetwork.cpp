#include "services/network/WiFiNetwork.hpp"
#include "gmock/gmock.h"

TEST(WiFiNetworkTest, WiFiSecurity_prints_correct_security_type)
{
    services::WiFiSecurity security;
    security.securityMode = services::WiFiSecurity::SecurityMode::unknown;

    ASSERT_EQ("unknown", security.ToString());
    ASSERT_EQ("open", services::WiFiSecurity{}.ToString());
    ASSERT_EQ("wep", services::WiFiSecurity::WepSecurity("").ToString());
    ASSERT_EQ("wpa", services::WiFiSecurity::WpaSecurity("").ToString());
    ASSERT_EQ("wpa-2", services::WiFiSecurity::Wpa2Security("").ToString());
    ASSERT_EQ("wpa-3", services::WiFiSecurity::Wpa3Security("").ToString());
}

TEST(WiFiNetworkTest, WiFiSecurity_is_comparable)
{
    ASSERT_TRUE(services::WiFiSecurity::WpaSecurity("12345678") == services::WiFiSecurity::WpaSecurity("12345678"));
    ASSERT_FALSE(services::WiFiSecurity::WpaSecurity("12345678") != services::WiFiSecurity::WpaSecurity("12345678"));

    ASSERT_FALSE(services::WiFiSecurity::WpaSecurity("") == services::WiFiSecurity::WpaSecurity("12345678"));
    ASSERT_TRUE(services::WiFiSecurity::WpaSecurity("") != services::WiFiSecurity::WpaSecurity("12345678"));

    ASSERT_FALSE(services::WiFiSecurity::Wpa2Security("12345678") == services::WiFiSecurity::WpaSecurity("12345678"));
    ASSERT_TRUE(services::WiFiSecurity::Wpa2Security("12345678") != services::WiFiSecurity::WpaSecurity("12345678"));
}
