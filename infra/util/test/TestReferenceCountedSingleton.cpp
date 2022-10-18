#include "infra/util/ReferenceCountedSingleton.hpp"
#include "gtest/gtest.h"

class ReferenceCountedSingletonTest
    : public testing::Test
{
public:
    ReferenceCountedSingletonTest()
    {
        numCreations = 0;
    }

    static int numCreations;

    struct X
        : public infra::ReferenceCountedSingleton<X>
    {
        X()
            : something(true)
        {
            ++numCreations;
        }

        ~X()
        {
            --numCreations;
        }

        bool something;
    };
};

int ReferenceCountedSingletonTest::numCreations = 0;

TEST_F(ReferenceCountedSingletonTest, TestCreation)
{
    EXPECT_EQ(numCreations, 0);
    X::Access access;
    EXPECT_EQ(numCreations, 1);
}

TEST_F(ReferenceCountedSingletonTest, TestDestruction)
{
    {
        X::Access access;
        EXPECT_EQ(numCreations, 1);
    }
    EXPECT_EQ(numCreations, 0);
}

TEST_F(ReferenceCountedSingletonTest, TestMultipleAccess)
{
    {
        X::Access access;
        {
            X::Access access2;
            EXPECT_EQ(numCreations, 1);
        }
        EXPECT_EQ(numCreations, 1);
    }
    EXPECT_EQ(numCreations, 0);
}

TEST_F(ReferenceCountedSingletonTest, TestDereference)
{
    X::Access access;
    EXPECT_TRUE(access->something);
    EXPECT_TRUE((*access).something);

    const X::Access constAccess;
    EXPECT_TRUE(constAccess->something);
    EXPECT_TRUE((*constAccess).something);
}
