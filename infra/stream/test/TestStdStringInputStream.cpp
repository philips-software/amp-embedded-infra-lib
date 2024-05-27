#include "infra/stream/StdStringInputStream.hpp"
#include "gtest/gtest.h"
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
    infra::StdStringInputStream::WithStorage stream(std::in_place, "", infra::softFail);

    uint8_t v(1);
    stream >> infra::hex >> v;
    EXPECT_EQ(0, v);
    EXPECT_TRUE(stream.Failed());
}

TEST(StdStringInputStreamTest, ExtractStringLiteral)
{
    infra::StdStringInputStream::WithStorage stream(std::in_place, "abcd");

    stream >> "abcd";
    EXPECT_TRUE(stream.Empty());
}

TEST(StdStringInputStreamTest, PeekLiteral)
{
    infra::StdStringInputStream::WithStorage stream(std::in_place, "a");

    EXPECT_EQ('a', stream.PeekContiguousRange(0).front());
    EXPECT_EQ("a", stream.Storage());

    stream.Consume(1);
    EXPECT_TRUE(stream.Empty());
}

TEST(StdStringInputStreamTest, PeekContiguousRange_returns_expected_range_size)
{
    infra::StdStringInputStream::WithStorage stream(std::in_place, "abcdef");

    EXPECT_EQ(6, stream.PeekContiguousRange().size());

    stream.Consume(2);
    EXPECT_EQ(4, stream.PeekContiguousRange().size());
}
