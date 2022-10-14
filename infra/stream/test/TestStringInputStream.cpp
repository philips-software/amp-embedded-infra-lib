#include "gtest/gtest.h"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include <cstdint>

TEST(StringInputStreamTest, ExtractDecimal)
{
    infra::BoundedString::WithStorage<10> string("12");
    infra::StringInputStream stream(string);

    uint8_t value;
    stream >> value;
    EXPECT_EQ(12, value);
}

TEST(StringInputStreamTest, ExtractHex)
{
    infra::BoundedString::WithStorage<10> string("ab");
    infra::StringInputStream stream(string);

    uint8_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xab, value);
}

TEST(StringInputStreamTest, ExtractSmallValue)
{
    infra::BoundedString::WithStorage<10> string("a");
    infra::StringInputStream stream(string);

    uint8_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xa, value);
}

TEST(StringInputStreamTest, ExtractDelimitedValue)
{
    infra::BoundedString::WithStorage<10> string("a ");
    infra::StringInputStream stream(string);

    uint8_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xa, value);
}

TEST(StringInputStreamTest, ExtractPartialValue)
{
    infra::BoundedString::WithStorage<10> string("abcd");
    infra::StringInputStream stream(string);

    uint8_t value;
    stream >> infra::hex >> infra::Width(1) >> value;
    EXPECT_EQ(0xa, value);
}

TEST(StringInputStreamTest, ExtractUint16)
{
    infra::BoundedString::WithStorage<10> string("abcd");
    infra::StringInputStream stream(string);

    uint16_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xabcd, value);
}

TEST(StringInputStreamTest, ExtractUint32)
{
    infra::BoundedString::WithStorage<10> string("abcd0123");
    infra::StringInputStream stream(string);

    uint32_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xabcd0123, value);
}

TEST(StringInputStreamTest, ExtractUint64)
{
    infra::BoundedString::WithStorage<17> string("abcd0123abcd0123");
    infra::StringInputStream stream(string);

    uint64_t value;
    stream >> infra::hex >> value;
    EXPECT_EQ(0xabcd0123abcd0123, value);
}

TEST(StringInputStreamTest, ExtractHexWithOverflow)
{
    infra::BoundedString::WithStorage<1> string("");
    infra::StringInputStream stream(string, infra::softFail);

    uint8_t v(1);
    stream >> infra::hex >> v;
    EXPECT_EQ(0, v);
    EXPECT_TRUE(stream.Failed());
}

TEST(StringInputStreamTest, ExtractHexWithoutGoodCharacters)
{
    infra::StringInputStream stream(infra::BoundedConstString("k"), infra::softFail);

    uint8_t v(1);
    stream >> infra::hex >> v;
    EXPECT_EQ(0, v);
    EXPECT_TRUE(stream.Failed());
}

TEST(StringInputStreamTest, Consume)
{
    infra::BoundedConstString string("abcd");
    infra::StringInputStream stream(string, infra::noFail);

    EXPECT_EQ(string, infra::ByteRangeAsString(stream.PeekContiguousRange(0)));
    stream.Consume(stream.PeekContiguousRange().size());
    EXPECT_EQ("", infra::ByteRangeAsString(stream.PeekContiguousRange(0)));
}

TEST(StringInputStreamTest, FromHex)
{
    infra::StringInputStream stream(infra::BoundedConstString("ab01"));

    std::array<uint8_t, 2> output;
    stream >> infra::FromHex(output);

    EXPECT_EQ((std::array<uint8_t, 2>{ 0xab, 0x01 }), output);
    EXPECT_TRUE(stream.Empty());
}

TEST(StringInputStreamTest, FromBase64)
{
    infra::StringInputStream stream(infra::BoundedConstString("YWJj"));

    std::array<uint8_t, 3> output;
    stream >> infra::FromBase64(output);

    EXPECT_EQ((std::array<uint8_t, 3>{ 'a', 'b', 'c' }), output);
    EXPECT_TRUE(stream.Empty());
}

TEST(StringInputStreamTest, FromBase64_with_one_pad)
{
    infra::StringInputStream stream(infra::BoundedConstString("YWI="));

    std::array<uint8_t, 2> output;
    stream >> infra::FromBase64(output);

    EXPECT_EQ((std::array<uint8_t, 2>{ 'a', 'b' }), output);
    EXPECT_TRUE(stream.Empty());
}

TEST(StringInputStreamTest, FromBase64_with_two_pads)
{
    infra::StringInputStream stream(infra::BoundedConstString("YQ=="));

    std::array<uint8_t, 1> output;
    stream >> infra::FromBase64(output);

    EXPECT_EQ((std::array<uint8_t, 1>{ 'a' }), output);
    EXPECT_TRUE(stream.Empty());
}

TEST(StringInputStreamTest, FromBase64_with_insufficient_input_reports_error)
{
    infra::StringInputStream stream(infra::BoundedConstString("YWJ"), infra::softFail);

    std::array<uint8_t, 3> output;
    stream >> infra::FromBase64(output);

    EXPECT_TRUE(stream.ErrorPolicy().Failed());
}

TEST(StringInputStreamTest, Base64DecodedSize)
{
    EXPECT_EQ(3, Base64DecodedSize(infra::BoundedConstString("YWJj")));
}

TEST(StringInputStreamTest, Base64DecodedSize_with_one_pads)
{
    EXPECT_EQ(2, Base64DecodedSize(infra::BoundedConstString("YWI=")));
}

TEST(StringInputStreamTest, Base64DecodedSize_with_two_pads)
{
    EXPECT_EQ(1, Base64DecodedSize(infra::BoundedConstString("YQ==")));
}

TEST(StringInputStreamTest, Base64DecodedSize_with_bigger_input)
{
    EXPECT_EQ(5, Base64DecodedSize(infra::BoundedConstString("YWJjZGU=")));
}

TEST(StringInputStreamTest, Base64DecodedSize_with_insufficient_input_reports_null)
{
    EXPECT_EQ(0, Base64DecodedSize(infra::BoundedConstString("YQ=")));
    EXPECT_EQ(0, Base64DecodedSize(infra::BoundedConstString("")));
}

TEST(StringInputStreamTest, Rewind)
{
    infra::BoundedString::WithStorage<10> string("abcd");
    infra::StringInputStream stream(string);

    auto marker = stream.Reader().ConstructSaveMarker();

    uint8_t value;
    stream >> infra::hex >> infra::Width(2) >> value;
    EXPECT_EQ(0xab, value);

    stream.Reader().Rewind(marker);

    stream >> infra::hex >> infra::Width(2) >> value;
    EXPECT_EQ(0xab, value);
}
