#include "gtest/gtest.h"
#include "infra/stream/OverwriteStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include <cstdint>
#include <limits>

namespace
{
    struct MyObject
    {
        explicit MyObject(int) {}
        MyObject(const MyObject& other) = delete;
        MyObject& operator=(const MyObject& other) = delete;

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const MyObject& object)
        {
            stream << "MyObject!";
            return stream;
        }
    };
}

TEST(StringOutputStreamTest, stream_byte)
{
    infra::StringOutputStream::WithStorage<2> stream;

    stream << uint8_t(12);
    EXPECT_EQ("12", stream.Storage());
}

TEST(StringOutputStreamTest, stream_from_bounded_string)
{
    infra::BoundedString::WithStorage<10> s = "abcd";
    infra::StringOutputStream stream(s);

    stream << uint8_t(12) << infra::data << uint8_t('a');

    EXPECT_EQ("abcd12a", s);
}

TEST(StringOutputStreamTest, stream_literal)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << "abcd";

    EXPECT_EQ("abcd", stream.Storage());
}

TEST(StringOutputStreamTest, stream_literal_in_hex_stream)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::hex << "abcd";

    EXPECT_EQ("abcd", stream.Storage());
}

TEST(StringOutputStreamTest, stream_literal_in_bin_stream)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::bin << "abcd";

    EXPECT_EQ("abcd", stream.Storage());
}

TEST(StringOutputStreamTest, stream_character)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << 'a';

    EXPECT_EQ("a", stream.Storage());
}

TEST(StringOutputStreamTest, stream_uint8)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << uint8_t(255);

    EXPECT_EQ("255", stream.Storage());
}

TEST(StringOutputStreamTest, stream_int8)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << int8_t(127);

    EXPECT_EQ("127", stream.Storage());
}

TEST(StringOutputStreamTest, stream_negative_int8)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << int8_t(-128);

    EXPECT_EQ("-128", stream.Storage());
}

TEST(StringOutputStreamTest, stream_int8_with_leading_zeroes)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::Width(5,'0') << int8_t(127);

    EXPECT_EQ("00127", stream.Storage());
}

TEST(StringOutputStreamTest, stream_int8_with_smaller_width)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::Width(2) << int8_t(127);

    EXPECT_EQ("127", stream.Storage());
}

TEST(StringOutputStreamTest, stream_negative_int8_with_width)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::Width(7) << int8_t(-127);

    EXPECT_EQ("   -127", stream.Storage());
}

TEST(StringOutputStreamTest, stream_uint64_max)
{
    infra::StringOutputStream::WithStorage<20> stream;

    stream << std::numeric_limits<uint64_t>::max();

    EXPECT_EQ("18446744073709551615", stream.Storage());
}

TEST(StringOutputStreamTest, stream_int64_max)
{
    infra::StringOutputStream::WithStorage<20> stream;

    stream << std::numeric_limits<int64_t>::max();

    EXPECT_EQ("9223372036854775807", stream.Storage());
}

TEST(StringOutputStreamTest, stream_int64_min)
{
    infra::StringOutputStream::WithStorage<20> stream;

    stream << std::numeric_limits<int64_t>::min();

    EXPECT_EQ("-9223372036854775808", stream.Storage());
}

TEST(StringOutputStreamTest, stream_float)
{
    infra::StringOutputStream::WithStorage<20> stream;

    stream << float(42.123);

    EXPECT_EQ("42.123", stream.Storage());
}

TEST(StringOutputStreamTest, stream_negative_float)
{
    infra::StringOutputStream::WithStorage<20> stream;

    stream << float(-42.123);

    EXPECT_EQ("-42.123", stream.Storage());
}

TEST(StringOutputStreamTest, stream_enum_value)
{
    infra::StringOutputStream::WithStorage<20> stream;

    enum E { a, b, c };
    E e = E::b;

    stream << e;

    EXPECT_EQ("1", stream.Storage());
}

TEST(StringOutputStreamTest, stream_short_hex)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::hex << uint8_t(10);

    EXPECT_EQ("a", stream.Storage());
}

TEST(StringOutputStreamTest, stream_longer_hex)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::hex << uint8_t(0x1A);
    EXPECT_EQ("1a", stream.Storage());
}

TEST(StringOutputStreamTest, stream_negative_hex)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::hex << int8_t(-0x1A);
    EXPECT_EQ("-1a", stream.Storage());
}

TEST(StringOutputStreamTest, stream_hex_with_leading_zeroes)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::hex << infra::Width(4,'0') << uint8_t(0x1A);
    EXPECT_EQ("001a", stream.Storage());
}

TEST(StringOutputStreamTest, stream_short_bin)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::bin << uint8_t(1);

    EXPECT_EQ("1", stream.Storage());
}

TEST(StringOutputStreamTest, stream_longer_bin)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::bin << uint8_t(0x1A);
    EXPECT_EQ("11010", stream.Storage());
}

TEST(StringOutputStreamTest, stream_negative_bin)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::bin << int8_t(-0x1A);
    EXPECT_EQ("-11010", stream.Storage());
}

TEST(StringOutputStreamTest, stream_bin_with_leading_zeroes)
{
    infra::StringOutputStream::WithStorage<10> stream;

    stream << infra::bin << infra::Width(8, '0') << uint8_t(0x1A);
    EXPECT_EQ("00011010", stream.Storage());
}

TEST(StringOutputStreamTest, overflow)
{
    infra::StringOutputStream::WithStorage<2> stream(infra::softFail);

    stream << "abc";
    EXPECT_EQ("ab", stream.Storage());
    EXPECT_TRUE(stream.Failed());
}

TEST(StringOutputStreamTest, overflow_with_noFail)
{
    infra::StringOutputStream::WithStorage<2> stream(infra::noFail);

    stream << "abc";
    EXPECT_EQ("ab", stream.Storage());
}

TEST(StringOutputStreamTest, overflow_twice)
{
    infra::StringOutputStream::WithStorage<2> stream(infra::softFail);

    stream << "abc" << "def";
    EXPECT_EQ("ab", stream.Storage());
    EXPECT_TRUE(stream.Failed());
}

TEST(StringOutputStreamTest, format_simple_string)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("simple");
    EXPECT_EQ("simple", stream.Storage());
}

TEST(StringOutputStreamTest, format_simple_string_width)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream << infra::Width(10) << "simple";
    EXPECT_EQ("    simple", stream.Storage());
}

TEST(StringOutputStreamTest, format_string_with_one_parameter)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("%1%", 1);
    EXPECT_EQ("1", stream.Storage());
}

TEST(StringOutputStreamTest, format_string_with_two_parameters)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("%1% %2%", 5, 'a');
    EXPECT_EQ("5 a", stream.Storage());
}

TEST(StringOutputStreamTest, format_string_with_string)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("%1%", "bla");
    EXPECT_EQ("bla", stream.Storage());
}

TEST(StringOutputStreamTest, format_string_with_invalid_param_specifier)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("%bla");
    EXPECT_EQ("bla", stream.Storage());
}

TEST(StringOutputStreamTest, format_string_with_incomplete_param_specifier)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("%1", "bla");
    EXPECT_EQ("", stream.Storage());
}

TEST(StringOutputStreamTest, format_string_with_out_of_range_index)
{
    infra::StringOutputStream::WithStorage<64> stream;

    stream.Format("%3%", 1, 2);
    EXPECT_EQ("", stream.Storage());
}

TEST(StringOutputStreamTest, format_custom_parameter)
{
    infra::StringOutputStream::WithStorage<64> stream;

    MyObject myObject(5);
    stream.Format("%1%", myObject);
    EXPECT_EQ("MyObject!", stream.Storage());
}

TEST(StringOutputStreamTest, stream_byte_range_as_ascii)
{
    infra::StringOutputStream::WithStorage<64> stream;

    std::array<uint8_t, 4> data = { 1, 2, 'a', 'b' };
    stream << infra::AsAscii(infra::ByteRange(data));
    EXPECT_EQ("..ab", stream.Storage());
}

TEST(StringOutputStreamTest, stream_byte_range_as_hex)
{
    infra::StringOutputStream::WithStorage<64> stream;

    std::array<uint8_t, 4> data = { 1, 2, 0x30, 0x40 };
    stream << infra::AsHex(infra::ByteRange(data));
    EXPECT_EQ("01023040", stream.Storage());
}

TEST(StringOutputStreamTest, stream_byte_range_as_base64)
{
    infra::StringOutputStream::WithStorage<64> stream1;
    stream1 << infra::AsBase64(std::array<uint8_t, 1>{ 'a' });
    EXPECT_EQ("YQ==", stream1.Storage());

    infra::StringOutputStream::WithStorage<64> stream2;
    stream2 << infra::AsBase64(std::array<uint8_t, 2>{ 'a', 'b' });
    EXPECT_EQ("YWI=", stream2.Storage());

    infra::StringOutputStream::WithStorage<64> stream3;
    stream3 << infra::AsBase64(std::array<uint8_t, 3>{ 'a', 'b', 'c' });
    EXPECT_EQ("YWJj", stream3.Storage());

    infra::StringOutputStream::WithStorage<64> stream4;
    stream4 << infra::AsBase64(std::array<uint8_t, 4>{ 'a', 'b', 'c', 'd' });
    EXPECT_EQ("YWJjZA==", stream4.Storage());

    infra::StringOutputStream::WithStorage<64> stream5;
    stream5 << infra::data << infra::text << infra::AsBase64(std::array<uint8_t, 1>{ 'a' });
    EXPECT_EQ("YQ==", stream5.Storage());
}

TEST(StringOutputStreamTest, stream_byte_range_as_combined_base64)
{

    infra::StringOutputStream::WithStorage<64> stream1;
    stream1 << infra::AsBase64({ infra::ConstByteRange(), std::array<uint8_t, 1>{ 'a' } });
    EXPECT_EQ("YQ==", stream1.Storage());

    infra::StringOutputStream::WithStorage<64> stream2;
    stream2 << infra::AsBase64({ std::array<uint8_t, 1>{ 'a' }, std::array<uint8_t, 1>{ 'b' } });
    EXPECT_EQ("YWI=", stream2.Storage());

    infra::StringOutputStream::WithStorage<64> stream3;
    stream3 << infra::AsBase64({ std::array<uint8_t, 1>{ 'a' }, std::array<uint8_t, 2>{ 'a', 'b' } });
    EXPECT_EQ("YWFi", stream3.Storage());

    infra::StringOutputStream::WithStorage<64> stream4;
    stream4 << infra::AsBase64({ std::array<uint8_t, 2>{ 'a', 'b' }, std::array<uint8_t, 2>{ 'c', 'd' } });
    EXPECT_EQ("YWJjZA==", stream4.Storage());

    infra::StringOutputStream::WithStorage<64> stream5;
    stream5 << infra::data << infra::text << infra::AsBase64({ std::array<uint8_t, 1>{ 'a' } });
    EXPECT_EQ("YQ==", stream5.Storage());
}

TEST(StringOutputStreamTest, reserve_type)
{
    infra::StringOutputStream::WithStorage<64> stream;
    stream << "a";
    auto reservedSpace = stream.Writer().Reserve<uint8_t>(stream.ErrorPolicy());
    stream << "c";
    reservedSpace = 'b';

    EXPECT_EQ("abc", stream.Storage());
}

TEST(StringOutputStreamTest, reserve_type_without_space)
{
    infra::StringOutputStream::WithStorage<2> stream(infra::softFail);
    stream << "a";
    auto reservedSpace = stream.Writer().Reserve<uint32_t>(stream.ErrorPolicy());
    reservedSpace = uint32_t(32);

    EXPECT_TRUE(stream.Failed());
}

TEST(StringOutputStreamTest, stream_to_saved_point)
{
    infra::StringOutputStream::WithStorage<64> stream;
    stream << "a";
    auto marker = stream.SaveMarker();
    stream << "c";

    {
        EXPECT_EQ(1, stream.ProcessedBytesSince(marker));
        infra::SavedMarkerTextStream savedStream(stream, marker);
        savedStream << 12345;
    }

    EXPECT_EQ("a12345c", stream.Storage());
}

TEST(StringOutputStreamTest, stream_to_nested_saved_point)
{
    infra::StringOutputStream::WithStorage<64> stream;
    stream << "a";
    auto marker = stream.SaveMarker();
    stream << "c";

    {
        infra::SavedMarkerTextStream savedStream(stream, marker);
        savedStream << 1;
        auto nestedMarker = savedStream.SaveMarker();
        savedStream << 56;

        {
            EXPECT_EQ(2, savedStream.ProcessedBytesSince(nestedMarker));
            infra::SavedMarkerTextStream nestedSavedStream(savedStream, nestedMarker);
            nestedSavedStream << 234;
        }
    }

    EXPECT_EQ("a123456c", stream.Storage());
}

TEST(StringOutputStreamTest, overwrite_on_saved_point)
{
    infra::StringOutputStream::WithStorage<64> stream;
    stream << "a";
    auto marker = stream.SaveMarker();
    stream << "b";
    stream << "c";

    {
        EXPECT_EQ(2, stream.ProcessedBytesSince(marker));
        infra::OverwriteTextStream overwriteStream(stream, marker);
        overwriteStream << "d";
    }

    EXPECT_EQ("adc", stream.Storage());
}

TEST(StringOutputStreamTest, available_retuns_remaining_space)
{
    infra::StringOutputStream::WithStorage<4> sos;
    EXPECT_EQ(4, sos.Available());

    sos << 1234;
    EXPECT_EQ(0, sos.Available());
}

TEST(StringOutputStreamTest, Reset)
{
    infra::StringOutputStream::WithStorage<2> stream;

    stream << uint8_t(12);
    stream.Writer().Reset();
    stream << uint8_t(14);

    EXPECT_EQ("14", stream.Storage());
}

TEST(StringOutputStreamTest, Reset_with_new_storage)
{
    infra::StringOutputStream::WithStorage<2> stream;

    stream << uint8_t(12);
    infra::BoundedString::WithStorage<4> newString;
    stream.Writer().Reset(newString);
    stream << uint8_t(14);

    EXPECT_EQ("14", newString);
}

TEST(StringOutputStreamTest, stream_join_range)
{
    infra::StringOutputStream::WithStorage<10> stream;
    std::array<infra::BoundedConstString, 3> array{"ab", "cd", "ef"};

    stream << infra::Join("; ", infra::MakeRange(array));

    EXPECT_EQ("ab; cd; ef", stream.Storage());
}

TEST(StringOutputStreamTest, stream_join_range_empty)
{
    infra::StringOutputStream::WithStorage<10> stream;
    std::array<infra::BoundedConstString, 0> array{};

    stream << infra::Join("; ", infra::MakeRange(array));

    EXPECT_EQ("", stream.Storage());
}

TEST(StringOutputStreamTest, stream_join_range_single_entry)
{
    infra::StringOutputStream::WithStorage<10> stream;
    std::array<infra::BoundedConstString, 1> array{"qwerty"};

    stream << infra::Join("; ", infra::MakeRange(array));

    EXPECT_EQ("qwerty", stream.Storage());
}

TEST(StringOutputStreamTest, stream_join_custom_range)
{
    std::array<const std::pair<infra::BoundedConstString, infra::BoundedConstString>, 3> array{
        std::pair{ "b1", "xyz1" },
        std::pair{ "b2", "xyz2" },
        std::pair{ "b3", "xyz3" }
    };
    infra::StringOutputStream::WithStorage<50> stream;

    stream << infra::Join(";", infra::MakeRange(array), [](auto& stream, const auto& obj) { stream << "'" << obj.first << "'?'" << obj.second << "'"; });

    EXPECT_EQ(R"('b1'?'xyz1';'b2'?'xyz2';'b3'?'xyz3')", stream.Storage());
}
