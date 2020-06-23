#include "interfaces/Can.hpp"
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
