#include "hal/interfaces/MacAddress.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "gtest/gtest.h"

TEST(MacAddressTest, toStream)
{
    infra::StringOutputStream::WithStorage<64> stream;
    const hal::MacAddress mac = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67 };
    stream << infra::AsMacAddress(mac);
    EXPECT_EQ("12:23:34:45:56:67", stream.Storage());
}

TEST(MacAddressTest, fromStream)
{
    infra::StringInputStream stream{ "98:87:76:65:54:43" };

    hal::MacAddress mac{};
    stream >> infra::ToMacAddress(mac);

    hal::MacAddress expected{ 0x98, 0x87, 0x76, 0x65, 0x54, 0x43 };
    EXPECT_EQ(mac, expected);
}

TEST(MacAddressTest, toStreamAsLittleEndianMacAddress)
{
    infra::StringOutputStream::WithStorage<64> stream;
    const hal::MacAddress mac = { 0x67, 0x56, 0x45, 0x34, 0x23, 0x12 };
    stream << infra::AsLittleEndianMacAddress(mac);
    EXPECT_EQ("12:23:34:45:56:67", stream.Storage());
}

TEST(MacAddressTest, fromStreamToLittleEndianMacAddress)
{
    infra::StringInputStream stream{ "12:23:34:45:56:67" };

    hal::MacAddress mac{};
    stream >> infra::ToLittleEndianMacAddress(mac);

    hal::MacAddress expected{ 0x67, 0x56, 0x45, 0x34, 0x23, 0x12 };
    EXPECT_EQ(mac, expected);
}
