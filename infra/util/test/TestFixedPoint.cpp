#include "infra/util/FixedPoint.hpp"
#include "gtest/gtest.h"

TEST(FixedPointTest, Construction)
{
    EXPECT_EQ(0, (infra::FixedPoint<int, 10>().AsUnscaled()));
    EXPECT_EQ(34, (infra::FixedPoint<int, 10>(infra::unscaled, 34).AsUnscaled()));
    EXPECT_EQ(341, (infra::FixedPoint<int, 10>(34, 1).AsUnscaled()));
    EXPECT_EQ(340, (infra::FixedPoint<int, 10>(infra::scaled, 34).AsUnscaled()));
}

TEST(FixedPointTest, IntegerPart)
{
    infra::FixedPoint<int, 10> f(infra::unscaled, 34);
    EXPECT_EQ(3, f.IntegerPart());
}

TEST(FixedPointTest, FractionalPart)
{
    infra::FixedPoint<int, 10> f(infra::unscaled, 34);
    EXPECT_EQ(4, f.FractionalPart());
}

TEST(FixedPointTest, Rounded)
{
    EXPECT_EQ(3, (infra::FixedPoint<int, 10>(infra::unscaled, 34).Rounded()));
    EXPECT_EQ(4, (infra::FixedPoint<int, 10>(infra::unscaled, 35).Rounded()));
    EXPECT_EQ(-3, (infra::FixedPoint<int, 10>(infra::unscaled, -34).Rounded()));
    EXPECT_EQ(-4, (infra::FixedPoint<int, 10>(infra::unscaled, -35).Rounded()));
}

TEST(FixedPointTest, Addition)
{
    infra::FixedPoint<int, 10> f1(infra::unscaled, 34);
    infra::FixedPoint<int, 10> f2(infra::unscaled, 15);

    EXPECT_EQ(49, (f1 + f2).AsUnscaled());

    f1 += f2;
    EXPECT_EQ(49, f1.AsUnscaled());
}

TEST(FixedPointTest, Subtraction)
{
    infra::FixedPoint<int, 10> f1(infra::unscaled, 34);
    infra::FixedPoint<int, 10> f2(infra::unscaled, 15);

    EXPECT_EQ(19, (f1 - f2).AsUnscaled());

    f1 -= f2;
    EXPECT_EQ(19, f1.AsUnscaled());
}

TEST(FixedPointTest, Multiplication)
{
    infra::FixedPoint<int, 10> f1(infra::unscaled, 34);
    infra::FixedPoint<int, 10> f2(infra::unscaled, 15);

    EXPECT_EQ(51, (f1 * f2).AsUnscaled());
    EXPECT_EQ(68, (f1 * 2).AsUnscaled());
    EXPECT_EQ(68, (2 * f1).AsUnscaled());

    f1 *= f2;
    EXPECT_EQ(51, f1.AsUnscaled());
    f1 *= 2;
    EXPECT_EQ(102, f1.AsUnscaled());
}

TEST(FixedPointTest, Division)
{
    infra::FixedPoint<int, 10> f1(infra::unscaled, 34);
    infra::FixedPoint<int, 10> f2(infra::unscaled, 15);

    EXPECT_EQ(22, (f1 / f2).AsUnscaled());

    f1 /= f2;
    EXPECT_EQ(22, f1.AsUnscaled());
    EXPECT_EQ(11, (f1 / 2).AsUnscaled());
    EXPECT_EQ(9, (2 / f1).AsUnscaled());
}

TEST(FixedPointTest, Negation)
{
    infra::FixedPoint<int, 10> f1(infra::unscaled, 34);
    EXPECT_EQ(-34, (-f1).AsUnscaled());
}

TEST(FixedPointTest, Comparisons)
{
    infra::FixedPoint<int, 10> f1(infra::unscaled, 30);
    infra::FixedPoint<int, 10> f2(infra::unscaled, 15);

    EXPECT_TRUE(f1 == f1);
    EXPECT_FALSE(f1 == f2);

    EXPECT_TRUE(f2 < f1);
    EXPECT_FALSE(f1 < f2);
    EXPECT_FALSE(f1 < f1);

    EXPECT_TRUE(f1 == 3);
    EXPECT_FALSE(f2 == 1);
    EXPECT_FALSE(f2 == 2);

    EXPECT_TRUE(f1 < 4);
    EXPECT_FALSE(f1 < 3);
    EXPECT_TRUE(f2 < 2);
    EXPECT_FALSE(f2 < 1);

    EXPECT_TRUE(f1 > 2);
    EXPECT_FALSE(f1 > 3);
    EXPECT_TRUE(f2 > 1);
    EXPECT_FALSE(f2 > 2);
}
