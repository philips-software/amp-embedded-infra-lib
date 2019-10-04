#include "gmock/gmock.h"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/syntax/ProtoParser.hpp"

TEST(ProtoParserTest, GetVarInt_from_a_single_byte)
{
    infra::ByteInputStream stream(std::array<uint8_t, 1>{ 5 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(5, parser.GetVarInt());
}

TEST(ProtoParserTest, GetVarInt_from_multiple_bytes)
{
    infra::ByteInputStream stream(std::array<uint8_t, 2>{ 0x85, 3 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(389, parser.GetVarInt());
}

TEST(ProtoParserTest, GetVarInt_largest)
{
    infra::ByteInputStream stream(std::array<uint8_t, 10>{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), parser.GetVarInt());
}

TEST(ProtoParserTest, GetFixed32)
{
    infra::ByteInputStream stream(std::array<uint8_t, 4>{ 1, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(1, parser.GetFixed32());
}

TEST(ProtoParserTest, GetFixed64)
{
    infra::ByteInputStream stream(std::array<uint8_t, 8>{ 1, 0, 0, 0, 0, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(1, parser.GetFixed64());
}

TEST(ProtoParserTest, GetField_on_var_int_returns_uint64_t)
{
    infra::ByteInputStream stream(std::array<uint8_t, 2>{ 1 << 3, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_EQ(5, field.first.Get<uint64_t>());
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, GetField_on_fixed64_returns_uint64_t)
{
    infra::ByteInputStream stream(std::array<uint8_t, 9>{ (1 << 3) | 1, 5, 0, 0, 0, 0, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_EQ(5, field.first.Get<uint64_t>());
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, GetField_on_fixed32_returns_uint32_t)
{
    infra::ByteInputStream stream(std::array<uint8_t, 5>{ (1 << 3) | 5, 5, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_EQ(5, field.first.Get<uint32_t>());
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, GetField_returns_string)
{
    infra::ByteInputStream::WithStorage<3> stream(infra::inPlace, std::array<uint8_t, 3>{ (1 << 3) | 2, 1, 'a' });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::BoundedString::WithStorage<10> string;
    field.first.Get<infra::ProtoLengthDelimited>().GetString(string);
    EXPECT_EQ("a", string);
}

TEST(ProtoParserTest, GetField_returns_bytes)
{
    infra::ByteInputStream::WithStorage<3> stream(infra::inPlace, std::array<uint8_t, 3>{ (1 << 3) | 2, 1, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::BoundedVector<uint8_t>::WithMaxSize<10> bytes;
    field.first.Get<infra::ProtoLengthDelimited>().GetBytes(bytes);
    EXPECT_EQ(5, bytes.front());
}

TEST(ProtoParserTest, GetField_returns_nested_object)
{
    infra::ByteInputStream stream(std::array<uint8_t, 4>{ (1 << 3) | 2, 2, 1 << 3, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::ProtoParser nestedParser = field.first.Get<infra::ProtoLengthDelimited>().Parser();
    infra::ProtoParser::Field nestedField = nestedParser.GetField();
    EXPECT_EQ(5, nestedField.first.Get<uint64_t>());
    EXPECT_EQ(1, nestedField.second);
}

TEST(ProtoParserTest, SkipEverything_skips_LengthDelimited)
{
    infra::ByteInputStream stream(std::array<uint8_t, 6>{ (1 << 3) | 2, 2, 1 << 3, 5, 1 << 3, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();

    field.first.Get<infra::ProtoLengthDelimited>().SkipEverything();
    
    field = parser.GetField();
    EXPECT_EQ(5, field.first.Get<uint64_t>());
    EXPECT_EQ(1, field.second);
}
