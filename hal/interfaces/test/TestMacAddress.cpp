#include "gtest/gtest.h"
#include "interfaces/MacAddress.hpp"
#include "stream/StringOutputStream.hpp"
#include "infra\stream\Formatter.hpp"

TEST(MacAddressTest, toStream)
{
    infra::StringOutputStream::WithStorage<64> stream;
    const hal::MacAddress mac = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
    stream << infra::AsMacAddress(mac);
    EXPECT_EQ("12:23:34:45:56:67", stream.Storage());
}

template<>
void infra::Formatter<hal::MacAddress>::Format(TextOutputStream& stream, FormatSpec& spec)
{
    auto prependColon = false;
    for (auto v : value)
    {
        if (prependColon)
            stream << ':';
        else
            prependColon = true;
        stream << infra::Format("{:02x}", v);
    }
}

TEST(MacAddressTest, Format)
{
    infra::StringOutputStream::WithStorage<60> stream(infra::softFail);
    hal::MacAddress mac = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67 };
    stream << infra::Format("Mac Address = {}", mac);
    EXPECT_FALSE(stream.ErrorPolicy().Failed());
    EXPECT_EQ("Mac Address = 12:23:34:45:56:67", stream.Storage());

}
