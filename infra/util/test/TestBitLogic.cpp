#include "infra/util/BitLogic.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(BitLogicTest, Bit)
{
    EXPECT_EQ(32, infra::Bit<uint32_t>(5));
}

TEST(BitLogicTest, SetBit)
{
    uint32_t x = 1;
    infra::SetBit(x, 5);
    EXPECT_EQ(33, x);
}

TEST(BitLogicTest, ClearBit)
{
    uint32_t x = 33;
    infra::ClearBit(x, 5);
    EXPECT_EQ(1, x);
}

TEST(BitLogicTest, ReplaceBit_bool)
{
    uint32_t x = 1;
    infra::ReplaceBit(x, true, 5);
    EXPECT_EQ(33, x);
}

TEST(BitLogicTest, ReplaceBit_value)
{
    uint32_t x = 1;
    infra::ReplaceBit(x, static_cast<uint32_t>(1), 5);
    EXPECT_EQ(33, x);
}

TEST(BitLogicTest, ReplaceBits)
{
    uint32_t x = 1;
    infra::ReplaceBits(x, 2, 2, 0);
    EXPECT_EQ(2, x);
}

TEST(BitLogicTest, IsBitSet)
{
    uint32_t x = 32;
    EXPECT_TRUE(infra::IsBitSet(x, 5));
}

TEST(BitLogicTest, GetBits)
{
    uint32_t x = 2;
    EXPECT_EQ(2, infra::GetBits(x, 1, 1));
}

TEST(BitLogicTest, AreAnyBitsSet)
{
    const uint32_t x = 0b10000001;
    const uint32_t y = 0b10000000;
    const uint32_t mask = 0b1001;

    EXPECT_THAT(infra::AreAnyBitsSet(x, mask), testing::IsTrue());
    EXPECT_THAT(infra::AreAnyBitsSet(y, mask), testing::IsFalse());
}

TEST(BitLogicTest, AreAllBitsSet)
{
    const uint32_t x = 0b10001001;
    const uint32_t y = 0b10001000;
    const uint32_t mask = 0b1001;

    EXPECT_THAT(infra::AreAllBitsSet(x, mask), testing::IsTrue());
    EXPECT_THAT(infra::AreAllBitsSet(y, mask), testing::IsFalse());
}
