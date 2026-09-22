#include "infra/util/InterfaceConnector.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class ITestSingleton
    : public infra::InterfaceConnector<ITestSingleton>
{
public:
    virtual void f() = 0;
};

class TestSingleton
    : public ITestSingleton
{
public:
    void f() override
    {}
};

TEST(InterfaceConnectorTest, TestConstruction)
{
    TestSingleton t;

    EXPECT_THAT(&ITestSingleton::Instance(), testing::Eq(&t));
}

TEST(InterfaceConnectorTest, InstanceIsOnlySetWhileSingletonIsAlive)
{
    EXPECT_THAT(ITestSingleton::InstanceSet(), testing::IsFalse());

    {
        TestSingleton t;

        EXPECT_THAT(ITestSingleton::InstanceSet(), testing::IsTrue());
        EXPECT_THAT(&ITestSingleton::Instance(), testing::Eq(&t));
    }

    EXPECT_THAT(ITestSingleton::InstanceSet(), testing::IsFalse());
}

#ifndef EMIL_MUTATION_TESTING
TEST(InterfaceConnectorTest, ConstructingSecondSingletonAborts)
{
    TestSingleton t;

    EXPECT_DEATH({ TestSingleton second; }, "");
}

TEST(InterfaceConnectorTest, AccessingInstanceWithoutConstructedSingletonAborts)
{
    EXPECT_THAT(ITestSingleton::InstanceSet(), testing::IsFalse());
    EXPECT_DEATH(ITestSingleton::Instance(), "");
}
#endif
