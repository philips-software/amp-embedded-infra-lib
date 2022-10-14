#include "gtest/gtest.h"
#include "infra/util/AutoResetMultiFunction.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

TEST(AutoResetMultiFunctionTest, IsInvokeable)
{
    infra::MockCallback<void()> m;
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f([&m]()
        { m.callback(); });

    EXPECT_CALL(m, callback());
    f();
}

TEST(AutoResetMultiFunctionTest, IsResetAfterInvoke)
{
    infra::MockCallback<void()> m;
    infra::AutoResetMultiFunction<void()>::And<void(bool)>::And<void(int)> f([&m]()
        { m.callback(); });

    EXPECT_CALL(m, callback());
    f();
    EXPECT_FALSE(f);
}

TEST(AutoResetMultiFunctionTest, ReAssignDuringInvoke)
{
    infra::MockCallback<void()> b;
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f;

    f = [&b, &f]()
    {
        f = [&b]()
        {
            b.callback();
        };
    };

    EXPECT_CALL(b, callback());

    f();
    f();
    EXPECT_FALSE(f);
}

TEST(AutoResetMultiFunctionTest, TestConstructedEmpty)
{
    infra::AutoResetMultiFunction<void()> f;
    EXPECT_FALSE(f);
}

TEST(AutoResetMultiFunctionTest, TestConstructedEmptyByNullptr)
{
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f(nullptr);
    EXPECT_FALSE(f);
}

TEST(AutoResetMultiFunctionTest, TestMoveAssign)
{
    infra::MockCallback<void()> m;
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f([&m]()
        { m.callback(); });
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f2;
    f2 = std::move(f);
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f2();
}

TEST(AutoResetMultiFunctionTest, TestAssignNullptr)
{
    infra::MockCallback<void()> m;
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f([&m]()
        { m.callback(); });
    f = nullptr;
    EXPECT_FALSE(f);
}

TEST(AutoResetMultiFunctionTest, TestClone)
{
    infra::MockCallback<void(int)> m;
    infra::AutoResetMultiFunction<void(int)> f([&m](int x)
        { m.callback(x); });

    EXPECT_CALL(m, callback(1));
    f.Clone()(1);
    EXPECT_TRUE(static_cast<bool>(f));
}

TEST(AutoResetMultiFunctionTest, TestReturnValue)
{
    infra::AutoResetMultiFunction<int()> f([]()
        { return 1; });
    EXPECT_EQ(1, f());
}

TEST(AutoResetMultiFunctionTest, TestParameter)
{
    infra::MockCallback<void(int)> m;
    infra::AutoResetMultiFunction<void(int)> f([&m](int x)
        { m.callback(x); });
    EXPECT_CALL(m, callback(1));
    f(1);
}

TEST(AutoResetMultiFunctionTest, TestCompareToNullptr)
{
    infra::AutoResetMultiFunction<void()>::And<void(bool)> f;

    EXPECT_EQ(f, nullptr);
    EXPECT_EQ(nullptr, f);

    infra::AutoResetMultiFunction<void()>::And<void(bool)> g([]() {});

    EXPECT_NE(g, nullptr);
    EXPECT_NE(nullptr, g);
}
