#include "infra/stream/Scanner.hpp"
#include "gtest/gtest.h"

class ScannerTest : public testing::Test
{
protected:
    template<typename... Args>
    static void CheckScanArguments(const char* input, const char* format, Args&... args)
    {
        infra::StringInputStream stream(input);
        stream >> infra::Scan(format, args...);
        EXPECT_FALSE(stream.ErrorPolicy().Failed());
    }

    template<class T>
    static void CheckLimits(const char* input)
    {
        T min{};
        T max{};
        CheckScanArguments(input, "{}..{}", min, max);
        EXPECT_EQ(std::numeric_limits<T>::min(), min);
        EXPECT_EQ(std::numeric_limits<T>::max(), max);
    }

    template<typename... Args>
    static void FailedScanArguments(const char* format, Args&&... args)
    {
        infra::StringInputStream::WithStorage<60> stream(infra::softFail);
        stream >> infra::Scan(format, args...);
        EXPECT_TRUE(stream.ErrorPolicy().Failed());
    }
};

TEST_F(ScannerTest, Simple)
{
    uint32_t value{};
    CheckScanArguments("10", "{}", value);
    EXPECT_EQ(10, value);
}

TEST_F(ScannerTest, MissingArgument)
{
    uint32_t value{};
    FailedScanArguments("", value);
}

TEST_F(ScannerTest, TooManyArguments)
{
    uint32_t v1{};
    uint32_t v2{};
    FailedScanArguments("{}", v1, v2);
}

TEST_F(ScannerTest, TooFewArguments)
{
    uint32_t value{};
    FailedScanArguments("{} {}", value);
}

TEST_F(ScannerTest, IntegerLimitedWidth)
{
    uint32_t value{};
    CheckScanArguments("12345", "{:2}", value);
    EXPECT_EQ(12, value);
}

TEST_F(ScannerTest, SkipLeadingSpaces)
{
    uint32_t value{};
    CheckScanArguments("  10", "{}", value);
    EXPECT_EQ(10, value);
}

TEST_F(ScannerTest, boolean)
{
    CheckLimits<bool>("false..true");
}

TEST_F(ScannerTest, booleanWrong)
{
    bool b{};
    FailedScanArguments("fals", b);
}

TEST_F(ScannerTest, booleanLeadingSpaces)
{
    bool b{};
    CheckScanArguments("  true", "{}", b);
    EXPECT_EQ(true, b);
}

TEST_F(ScannerTest, int_uint8)
{
    CheckLimits<uint8_t>("0..255");
}

TEST_F(ScannerTest, int_int8)
{
    CheckLimits<int8_t>("-128..127");
}

TEST_F(ScannerTest, int_uint16)
{
    CheckLimits<uint16_t>("0..4294967295");
}

TEST_F(ScannerTest, int_int16)
{
    CheckLimits<int16_t>("-32768..32767");
}

TEST_F(ScannerTest, int_int32)
{
    CheckLimits<int32_t>("-2147483648..2147483647");
}

TEST_F(ScannerTest, int_uint32)
{
    CheckLimits<uint32_t>("0..4294967295");
}

TEST_F(ScannerTest, int_uint64)
{
    CheckLimits<uint64_t>("0..18446744073709551615");
}

TEST_F(ScannerTest, int_int64)
{
    CheckLimits<int64_t>("-9223372036854775808..9223372036854775807");
}

TEST_F(ScannerTest, integer_bin)
{
    uint32_t value{};
    CheckScanArguments("11100", "{:b}", value);
    EXPECT_EQ(0b11100, value);
}

TEST_F(ScannerTest, integer_oct)
{
    uint32_t value{};
    CheckScanArguments("1234", "{:o}", value);
    EXPECT_EQ(01234, value);
}

TEST_F(ScannerTest, integer_hex)
{
    uint32_t value{};
    CheckScanArguments("1A", "{:x}", value);
    EXPECT_EQ(0x1A, value);
}

TEST_F(ScannerTest, boundedString)
{
    infra::BoundedString::WithStorage<5> v;
    CheckScanArguments("hello", "{}", v);
    EXPECT_EQ("hello", v);
}

TEST_F(ScannerTest, boundedStringWithWidth)
{
    const infra::BoundedString::WithStorage<5> v;
    infra::BoundedString value(v);
    CheckScanArguments("hello", "{:2}", value);
    EXPECT_EQ("he", value);
}

