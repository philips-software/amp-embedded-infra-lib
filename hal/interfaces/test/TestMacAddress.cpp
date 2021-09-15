#include "interfaces/MacAddress.hpp"
#include "stream/StringOutputStream.hpp"
#include "gtest/gtest.h"

TEST(MacAddressTest, toStream)
{
    infra::StringOutputStream::WithStorage<64> stream;
    const hal::MacAddress mac = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67 };
    stream << infra::AsMacAddress(mac);
    EXPECT_EQ("12:23:34:45:56:67", stream.Storage());
}
