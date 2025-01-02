#include "hal/interfaces/Can.hpp"
#include "gtest/gtest.h"

TEST(CanTest, generate_11_bit_id)
{
    auto id = hal::Can::Id::Create11BitId(0);

    EXPECT_TRUE(id.Is11BitId());
    EXPECT_EQ(0, id.Get11BitId());
}

TEST(CanTest, generate_29_bit_id)
{
    auto id = hal::Can::Id::Create29BitId(0);

    EXPECT_TRUE(id.Is29BitId());
    EXPECT_EQ(0, id.Get29BitId());
}

TEST(CanTest, test_equality_mixed)
{
    auto id11_0 = hal::Can::Id::Create11BitId(0);
    auto id11_1 = hal::Can::Id::Create11BitId(1);
    auto id29_0 = hal::Can::Id::Create29BitId(0);
    auto id29_1 = hal::Can::Id::Create29BitId(1);

    EXPECT_TRUE(id11_0 == hal::Can::Id::Create11BitId(0));
    EXPECT_TRUE(id11_0 != id11_1);
    EXPECT_TRUE(id11_0 != id29_0);
    EXPECT_TRUE(id11_0 != id29_1);

    EXPECT_TRUE(id11_1 != id11_0);
    EXPECT_TRUE(id11_1 == hal::Can::Id::Create11BitId(1));
    EXPECT_TRUE(id11_1 != id29_0);
    EXPECT_TRUE(id11_1 != id29_1);

    EXPECT_TRUE(id29_0 != id11_0);
    EXPECT_TRUE(id29_0 != id11_1);
    EXPECT_TRUE(id29_0 == hal::Can::Id::Create29BitId(0));
    EXPECT_TRUE(id29_0 != id29_1);

    EXPECT_TRUE(id29_1 != id11_0);
    EXPECT_TRUE(id29_1 != id11_1);
    EXPECT_TRUE(id29_1 != id29_0);
    EXPECT_TRUE(id29_1 == hal::Can::Id::Create29BitId(1));
}
