#include "infra/util/Endian.hpp"
#include "gtest/gtest.h"

TEST(EndianTest, SwapEndian_changes_endianness)
{
    EXPECT_EQ(0x12345678, infra::SwapEndian(0x78563412));
}

TEST(EndianTest, BigEndian_stores_value_in_other_endianness)
{
    infra::BigEndian<uint16_t> value(0x1234);

    EXPECT_EQ(0x3412, reinterpret_cast<uint16_t&>(value));
}

TEST(EndianTest, compare_BigEndian_objects)
{
    infra::BigEndian<uint16_t> defaultConstructed;
    infra::BigEndian<uint16_t> x(1);

    EXPECT_EQ(1, static_cast<uint16_t>(x));
    EXPECT_TRUE(x == x);
    EXPECT_FALSE(x != x);
    EXPECT_TRUE(x != defaultConstructed);
    EXPECT_TRUE(x == static_cast<uint16_t>(1));
    EXPECT_TRUE(static_cast<uint16_t>(1) == x);
    EXPECT_TRUE(x != static_cast<uint16_t>(2));
    EXPECT_TRUE(static_cast<uint16_t>(2) != x);
}

TEST(EndianTest, array_in_BigEndian)
{
    infra::BigEndian<std::array<uint8_t, 5>> x{ { 1, 2, 3, 4, 5 } };
    EXPECT_EQ((std::array<uint8_t, 5>{ 5, 4, 3, 2, 1 }), (reinterpret_cast<std::array<uint8_t, 5>&>(x)));
}

TEST(EndianTest, conversion_functions)
{
    EXPECT_EQ(0x3412, infra::FromBigEndian(static_cast<uint16_t>(0x1234)));
    EXPECT_EQ(0x3412, infra::ToBigEndian(static_cast<uint16_t>(0x1234)));
    EXPECT_EQ(0x1234, infra::FromLittleEndian(static_cast<uint16_t>(0x1234)));
    EXPECT_EQ(0x1234, infra::ToLittleEndian(static_cast<uint16_t>(0x1234)));
}
