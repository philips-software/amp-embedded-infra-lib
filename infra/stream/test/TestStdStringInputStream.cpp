#include "gtest/gtest.h"
#include "infra/stream/StdStringInputStream.hpp"
#include <cstdint>

TEST(StdStringInputStreamTest, ExtractHex)
{
    std::string string("ab");
    infra::StdStringInputStream stream(string);

    uint8_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xab, value);
}

TEST(StdStringInputStreamTest, StdStringInputStream)
{
    std::string string("abcd");
    infra::StdStringInputStream stream(string);

    uint8_t value;
    stream >> infra::hex >> infra::Width(1) >> value;
    EXPECT_EQ(0xa, value);
}

TEST(StdStringInputStreamTest, ExtractHexFrowStdStringInputStreamWithOverflow)
{
    infra::StdStringInputStream::WithStorage stream(infra::inPlace, "", infra::softFail);

    uint8_t v(1);
    stream >> infra::hex >> v;
    EXPECT_EQ(0, v);
    EXPECT_TRUE(stream.Failed());
}

TEST(StdStringInputStreamTest, ExtractStringLiteral)
{
    infra::StdStringInputStream::WithStorage stream(infra::inPlace, "abcd");

    stream >> "abcd";
    EXPECT_TRUE(stream.Empty());
}

TEST(StdStringInputStreamTest, PeekLiteral)
{
    infra::StdStringInputStream::WithStorage stream(infra::inPlace, "a");

    EXPECT_EQ('a', stream.PeekContiguousRange(0).front());
    EXPECT_EQ("a", stream.Storage());

    stream.Consume(1);
    EXPECT_TRUE(stream.Empty());
}
