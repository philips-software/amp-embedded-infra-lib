#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(AutoResetFunctionTest, IsInvokeable)
{
    infra::MockCallback<void()> m;
    infra::AutoResetFunction<void()> f([&m]()
        { m.callback(); });

    EXPECT_CALL(m, callback());
    f();
}

TEST(AutoResetFunctionTest, IsResetAfterInvoke)
{
    infra::MockCallback<void()> m;
    infra::AutoResetFunction<void()> f([&m]()
        { m.callback(); });

    EXPECT_CALL(m, callback());
    f();
    EXPECT_FALSE(f);
}

TEST(AutoResetFunctionTest, ReAssignDuringInvoke)
{
    infra::MockCallback<void()> a;
    infra::MockCallback<void()> b;
    infra::AutoResetFunction<void(), 3 * sizeof(void*)> f;
    f = [&a, &b, &f]()
    {
        f = [&b]()
        {
            b.callback();
        };
        a.callback();
    };

    EXPECT_CALL(a, callback());
    EXPECT_CALL(b, callback());

    f();
    f();
    EXPECT_FALSE(f);
}

TEST(AutoResetFunctionTest, TestConstructedEmpty)
{
    infra::AutoResetFunction<void()> f;
    EXPECT_FALSE(f);
}

TEST(AutoResetFunctionTest, TestConstructedEmptyByNullptr)
{
    infra::AutoResetFunction<void()> f(nullptr);
    EXPECT_FALSE(f);
}

TEST(AutoResetFunctionTest, TestMoveAssign)
{
    infra::MockCallback<void()> m;
    infra::AutoResetFunction<void()> f([&m]()
        { m.callback(); });
    infra::AutoResetFunction<void()> f2;
    f2 = std::move(f);
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f2();
}

TEST(AutoResetFunctionTest, TestAssignNullptr)
{
    infra::MockCallback<void()> m;
    infra::AutoResetFunction<void()> f([&m]()
        { m.callback(); });
    f = nullptr;
    EXPECT_FALSE(f);
}

TEST(AutoResetFunctionTest, TestInvoke0)
{
    infra::MockCallback<void()> m;
    infra::AutoResetFunction<void()> f([&m]()
        { m.callback(); });
    EXPECT_CALL(m, callback());
    f.Invoke(std::make_tuple());
}

TEST(AutoResetFunctionTest, TestInvoke1)
{
    infra::MockCallback<void(int)> m;
    infra::AutoResetFunction<void(int)> f([&m](int x)
        { m.callback(x); });
    EXPECT_CALL(m, callback(1));
    f.Invoke(std::make_tuple(1));
}

TEST(AutoResetFunctionTest, TestClone)
{
    infra::MockCallback<void(int)> m;
    infra::AutoResetFunction<void(int)> f([&m](int x)
        { m.callback(x); });

    EXPECT_CALL(m, callback(1));
    f.Clone().Invoke(std::make_tuple(1));
    EXPECT_TRUE(static_cast<bool>(f));
}

TEST(AutoResetFunctionTest, TestSwap)
{
    infra::MockCallback<void()> m;
    infra::AutoResetFunction<void()> f([&m]()
        { m.callback(); });
    infra::AutoResetFunction<void()> f2;
    swap(f, f2);
    EXPECT_FALSE(f);
    EXPECT_CALL(m, callback());
    f2();
}

TEST(AutoResetFunctionTest, TestReturnValue)
{
    infra::AutoResetFunction<int()> f([]()
        { return 1; });
    EXPECT_EQ(1, f());
}

TEST(AutoResetFunctionTest, TestParameter)
{
    infra::MockCallback<void(int)> m;
    infra::AutoResetFunction<void(int)> f([&m](int x)
        { m.callback(x); });
    EXPECT_CALL(m, callback(1));
    f(1);
}

TEST(AutoResetFunctionTest, TestCompareToNullptr)
{
    infra::AutoResetFunction<void()> f;

    EXPECT_EQ(f, nullptr);
    EXPECT_EQ(nullptr, f);

    infra::AutoResetFunction<void()> g([]() {});

    EXPECT_NE(g, nullptr);
    EXPECT_NE(nullptr, g);
}
