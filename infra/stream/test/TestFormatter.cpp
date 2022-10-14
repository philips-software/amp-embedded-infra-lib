#include "gtest/gtest.h"
#include "infra/stream/Formatter.hpp"

class FormatTest : public testing::Test
{
protected:
    template<typename... Args>
    static void CheckFormatArguments(const char* expected, const char* format, Args&&... args)
    {
        infra::StringOutputStream::WithStorage<60> stream(infra::softFail);
        stream << infra::Format(format, args...);
        EXPECT_FALSE(stream.ErrorPolicy().Failed());
        EXPECT_EQ(expected, stream.Storage());
    }

    template<class T>
    static void CheckLimits(const char* expected)
    {
        CheckFormatArguments(expected, "{}..{}", std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    template<typename... Args>
    static void FailedFormatArguments(const char* format, Args&&... args)
    {
        infra::StringOutputStream::WithStorage<60> stream(infra::softFail);
        stream << infra::Format(format, args...);
        EXPECT_TRUE(stream.ErrorPolicy().Failed());
    }
};

TEST_F(FormatTest, empty_string)
{
    CheckFormatArguments("", "");
}

TEST_F(FormatTest, simple_string)
{
    CheckFormatArguments("a", "a");
}

TEST_F(FormatTest, missing_argument)
{
    FailedFormatArguments("{}");
}

TEST_F(FormatTest, out_of_index)
{
    FailedFormatArguments("{1}");
}

TEST_F(FormatTest, string_width)
{
    CheckFormatArguments("b    ", "{:5}", "b");
}

TEST_F(FormatTest, boolean)
{
    CheckLimits<bool>("false..true");
}

TEST_F(FormatTest, boolean_center)
{
    CheckFormatArguments("    false     ", "{:^14}", false);
}

TEST_F(FormatTest, integer0)
{
    CheckFormatArguments("0", "{0}", 0);
}

TEST_F(FormatTest, int_uint8)
{
    CheckLimits<uint8_t>("0..255");
}

TEST_F(FormatTest, int_int8)
{
    CheckLimits<int8_t>("-128..127");
}

TEST_F(FormatTest, int_uint16)
{
    CheckLimits<uint16_t>("0..65535");
}

TEST_F(FormatTest, int_int16)
{
    CheckLimits<int16_t>("-32768..32767");
}

TEST_F(FormatTest, int_uint32)
{
    CheckLimits<uint32_t>("0..4294967295");
}

TEST_F(FormatTest, int_int32)
{
    CheckLimits<int32_t>("-2147483648..2147483647");
}

TEST_F(FormatTest, int_uint64)
{
    CheckLimits<uint64_t>("0..18446744073709551615");
}

TEST_F(FormatTest, int_int64)
{
    CheckLimits<int64_t>("-9223372036854775808..9223372036854775807");
}

TEST_F(FormatTest, integer12)
{
    CheckFormatArguments("12", "{0}", 12);
}

TEST_F(FormatTest, integer12_width_zero_padding)
{
    CheckFormatArguments("00012", "{0:05}", 12);
}

TEST_F(FormatTest, integer12_width_tilde_padding)
{
    CheckFormatArguments("~~~12", "{0:~>05}", 12);
}

TEST_F(FormatTest, integer12_width_right)
{
    CheckFormatArguments("   12", "{0:5}", 12);
}

TEST_F(FormatTest, integer12_extra_width)
{
    CheckFormatArguments("    12     ", "{:^11}", 12);
}

TEST_F(FormatTest, integer12_width_left)
{
    CheckFormatArguments("12   ", "{0:<5}", 12);
}

TEST_F(FormatTest, integer12_width_center_fill)
{
    CheckFormatArguments("~12~~", "{0:~^5}", 12);
}

TEST_F(FormatTest, integer12_width_center_fill_extra)
{
    CheckFormatArguments("~~~~~~~12~~~~~~~", "{0:~^16}", 12);
}

TEST_F(FormatTest, integer12_width_center)
{
    CheckFormatArguments(" 12  ", "{:^5}", 12);
}

TEST_F(FormatTest, integer12no_index)
{
    CheckFormatArguments("12", "{}", 12);
}

TEST_F(FormatTest, multi_no_index)
{
    CheckFormatArguments("Twelve=12", "{}={}", "Twelve", 12);
}

TEST_F(FormatTest, integer_wrong_type)
{
    FailedFormatArguments("{0:t}", 0);
}

TEST_F(FormatTest, integer_oct)
{
    CheckFormatArguments("123456=0361100", "{0}=0{0:o}", 123456);
}

TEST_F(FormatTest, integer_hex)
{
    CheckFormatArguments("123456=0x1e240", "{0}=0x{0:x}", 123456);
}

TEST_F(FormatTest, integer_HEX)
{
    CheckFormatArguments("123456=0X1E240", "{0}=0X{0:X}", 123456);
}

TEST_F(FormatTest, integer_bin)
{
    CheckFormatArguments("123456=0b11110001001000000", "{0}=0b{0:b}", 123456);
}

TEST_F(FormatTest, integer_dec_oct_hex_bin)
{
    CheckFormatArguments("123456=0361100=0x1e240=0X1E240=0b11110001001000000", "{0}=0{:o}=0x{0:x}=0X{0:X}=0b{0:b}", 123456);
}

TEST_F(FormatTest, integer_large)
{
    CheckFormatArguments("12345678", "{0}", 12345678);
}

TEST_F(FormatTest, one_two_reverse)
{
    CheckFormatArguments("two one", "{1} {0}", "one", "two");
}

TEST_F(FormatTest, ten_plus_arguments)
{
    CheckFormatArguments("0 1 2 3 4 5 6 7 8 9 10 11 12", "{0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12}", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
}

namespace
{
    class TestFormatObject
    {
    public:
        TestFormatObject() = default;
        TestFormatObject(const TestFormatObject&) = delete;
        TestFormatObject& operator=(const TestFormatObject&) = delete;

        TestFormatObject(TestFormatObject&& other) noexcept
        {}

        TestFormatObject& operator=(TestFormatObject&& other) noexcept
        {
            return *this;
        }

        ~TestFormatObject() = default;

        int v{ 9 };
        bool b{ true };
    };
}

namespace infra
{
    template<>
    void Formatter<TestFormatObject const&>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        stream << infra::Format("v:{},b:{}", value.v, value.b);
    }
}

TEST_F(FormatTest, FormatObjectSimple)
{
    CheckFormatArguments("v:9,b:true", "{}", TestFormatObject());
}

TEST_F(FormatTest, FormatConstObject)
{
    const auto object = TestFormatObject();
    CheckFormatArguments("v:9,b:true", "{}", object);
}

TEST_F(FormatTest, FormatObjectCombined)
{
    CheckFormatArguments("Hello ..1.. 2 1 v:9,b:true", "Hello {0:.^5} {1} {0} {2}", 1, 2, TestFormatObject());
}
