#include "hal/interfaces/Can.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

static_assert(hal::Can::Id::Create11BitId(0).Is11BitId());
static_assert(!hal::Can::Id::Create11BitId(0).Is29BitId());
static_assert(hal::Can::Id::Create11BitId(0).Get11BitId() == 0);

static_assert(!hal::Can::Id::Create29BitId(0).Is11BitId());
static_assert(hal::Can::Id::Create29BitId(0).Is29BitId());
static_assert(hal::Can::Id::Create29BitId(0).Get29BitId() == 0);

static_assert(hal::Can::Id::Create11BitId(0) == hal::Can::Id::Create11BitId(0));
static_assert(hal::Can::Id::Create11BitId(0) != hal::Can::Id::Create11BitId(1));
static_assert(hal::Can::Id::Create11BitId(0) != hal::Can::Id::Create29BitId(0));

static_assert(hal::Can::Id::Create29BitId(2) == hal::Can::Id::Create29BitId(2));
static_assert(hal::Can::Id::Create29BitId(2) != hal::Can::Id::Create29BitId(1));
static_assert(hal::Can::Id::Create29BitId(2) != hal::Can::Id::Create11BitId(2));

TEST(CanTest, generate_11_bit_id)
{
    const auto id = hal::Can::Id::Create11BitId(0);

    EXPECT_THAT(id.Is11BitId(), testing::IsTrue());
    EXPECT_THAT(id.Is29BitId(), testing::IsFalse());
    EXPECT_THAT(id.Get11BitId(), testing::Eq(0));
}

TEST(CanTest, generate_29_bit_id)
{
    const auto id = hal::Can::Id::Create29BitId(0);

    EXPECT_THAT(id.Is29BitId(), testing::IsTrue());
    EXPECT_THAT(id.Is11BitId(), testing::IsFalse());
    EXPECT_THAT(id.Get29BitId(), testing::Eq(0));
}

TEST(CanTest, test_equality_mixed)
{
    const auto id11_0 = hal::Can::Id::Create11BitId(0);
    const auto id11_1 = hal::Can::Id::Create11BitId(1);
    const auto id29_0 = hal::Can::Id::Create29BitId(0);
    const auto id29_1 = hal::Can::Id::Create29BitId(1);

    EXPECT_THAT(id11_0, testing::Eq(hal::Can::Id::Create11BitId(0)));
    EXPECT_THAT(id11_0, testing::Ne(id11_1));
    EXPECT_THAT(id11_0, testing::Ne(id29_0));
    EXPECT_THAT(id11_0, testing::Ne(id29_1));

    EXPECT_THAT(id11_1, testing::Ne(id11_0));
    EXPECT_THAT(id11_1, testing::Eq(hal::Can::Id::Create11BitId(1)));
    EXPECT_THAT(id11_1, testing::Ne(id29_0));
    EXPECT_THAT(id11_1, testing::Ne(id29_1));

    EXPECT_THAT(id29_0, testing::Ne(id11_0));
    EXPECT_THAT(id29_0, testing::Ne(id11_1));
    EXPECT_THAT(id29_0, testing::Eq(hal::Can::Id::Create29BitId(0)));
    EXPECT_THAT(id29_0, testing::Ne(id29_1));

    EXPECT_THAT(id29_1, testing::Ne(id11_0));
    EXPECT_THAT(id29_1, testing::Ne(id11_1));
    EXPECT_THAT(id29_1, testing::Ne(id29_0));
    EXPECT_THAT(id29_1, testing::Eq(hal::Can::Id::Create29BitId(1)));
}
