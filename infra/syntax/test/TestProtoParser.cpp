#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "gmock/gmock.h"

TEST(ProtoParserTest, GetVarInt_from_a_single_byte)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 5 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(5, parser.GetVarInt());
}

TEST(ProtoParserTest, GetVarInt_from_multiple_bytes)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 0x85, 3 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(389, parser.GetVarInt());
}

TEST(ProtoParserTest, GetVarInt_largest)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), parser.GetVarInt());
}

TEST(ProtoParserTest, GetFixed32)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 1, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(1, parser.GetFixed32());
}

TEST(ProtoParserTest, GetFixed64)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 1, 0, 0, 0, 0, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    EXPECT_EQ(1, parser.GetFixed64());
}

TEST(ProtoParserTest, GetField_on_var_int_returns_uint64_t)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 1 << 3, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_EQ(5, std::get<uint64_t>(field.first));
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, GetField_on_fixed64_returns_uint64_t)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 1, 5, 0, 0, 0, 0, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_EQ(5, std::get<uint64_t>(field.first));
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, GetField_on_fixed32_returns_uint32_t)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 5, 5, 0, 0, 0 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_EQ(5, std::get<uint32_t>(field.first));
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, GetField_returns_string)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 1, 'a' });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::BoundedString::WithStorage<10> string;
    std::get<infra::ProtoLengthDelimited>(field.first).GetString(string);
    EXPECT_EQ("a", string);
}

TEST(ProtoParserTest, GetField_on_unknown_type_reports_error)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ 1 << 3 | 4, 5 }, infra::softFail);
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    EXPECT_TRUE(stream.Failed());
}

TEST(ProtoParserTest, too_short_string_is_reported_on_stream)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 5, 'a', 'b', 'c' }, infra::softFail);
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::BoundedString::WithStorage<10> string;
    std::get<infra::ProtoLengthDelimited>(field.first).GetString(string);
    EXPECT_TRUE(stream.Failed());
}

TEST(ProtoParserTest, GetField_returns_bytes)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 1, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::BoundedVector<uint8_t>::WithMaxSize<10> bytes;
    std::get<infra::ProtoLengthDelimited>(field.first).GetBytes(bytes);
    EXPECT_EQ(5, bytes.front());
}

TEST(ProtoParserTest, too_short_bytes_is_reported_on_stream)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 5, 5, 6, 7 }, infra::softFail);
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::BoundedVector<uint8_t>::WithMaxSize<10> bytes;
    std::get<infra::ProtoLengthDelimited>(field.first).GetBytes(bytes);
    EXPECT_TRUE(stream.Failed());
}

TEST(ProtoParserTest, GetField_returns_nested_object)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 2, 1 << 3, 5 });
    infra::ProtoParser parser(stream);

    infra::ProtoParser::Field field = parser.GetField();
    infra::ProtoParser nestedParser = std::get<infra::ProtoLengthDelimited>(field.first).Parser();
    infra::ProtoParser::Field nestedField = nestedParser.GetField();
    EXPECT_EQ(5, std::get<uint64_t>(nestedField.first));
    EXPECT_EQ(1, nestedField.second);
}

TEST(ProtoParserTest, SkipEverything_skips_LengthDelimited)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 2, 1 << 3, 5, 1 << 3, 5 });
    infra::ProtoParser parser(stream);

    {
        auto field{ parser.GetField() };

        std::get<infra::ProtoLengthDelimited>(field.first).SkipEverything();
    }

    auto field{ parser.GetField() };
    EXPECT_EQ(5, std::get<uint64_t>(field.first));
    EXPECT_EQ(1, field.second);
}

TEST(ProtoParserTest, ReportResult_is_propagated)
{
    infra::StdVectorInputStream::WithStorage stream(std::in_place, std::vector<uint8_t>{ (1 << 3) | 2, 5, 5, 6, 7 }, infra::softFail);
    infra::ProtoParser parser(stream);

    parser.ReportFormatResult(false);
    EXPECT_TRUE(stream.Failed());
}
