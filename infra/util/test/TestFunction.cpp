#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(FunctionTest, TestConstructedEmpty)
{
    infra::Function<void()> f;
    EXPECT_FALSE(f);
}

TEST(FunctionTest, TestConstructedEmptyByNullptr)
{
    infra::Function<void()> f(nullptr);
    EXPECT_FALSE(f);
}

TEST(FunctionTest, TestConstructedNotEmpty)
{
    infra::Function<void()> f([]() {});
    EXPECT_TRUE(static_cast<bool>(f));
}

void function()
{}

TEST(FunctionTest, TestConstructedWithFunctionPointer)
{
    infra::Function<void()> f(function);
    EXPECT_TRUE(static_cast<bool>(f));
    infra::Function<void()> g(f);
}

TEST(FunctionTest, TestConstructedWithTheEmptyFunction)
{
    infra::Function<void()> f(infra::emptyFunction);
    EXPECT_TRUE(static_cast<bool>(f));
    f();
}

TEST(FunctionTest, TestCall)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    EXPECT_CALL(m, callback());
    f();
}

TEST(FunctionTest, TestInvoke0)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    EXPECT_CALL(m, callback());
    f.Invoke(std::make_tuple());
}

TEST(FunctionTest, TestInvoke1)
{
    infra::MockCallback<void(int)> m;
    infra::Function<void(int)> f([&m](int x) { m.callback(x); });
    EXPECT_CALL(m, callback(1));
    f.Invoke(std::make_tuple(1));
}

TEST(FunctionTest, TestInvoke2)
{
    infra::MockCallback<void(int, int)> m;
    infra::Function<void(int, int)> f([&m](int x, int y) { m.callback(x, y); });
    EXPECT_CALL(m, callback(1, 2));
    f.Invoke(std::make_tuple(1, 2));
}

TEST(FunctionTest, TestInvoke3)
{
    infra::MockCallback<void(int, int, int)> m;
    infra::Function<void(int, int, int)> f([&m](int x, int y, int z) { m.callback(x, y, z); });
    EXPECT_CALL(m, callback(1, 2, 3));
    f.Invoke(std::make_tuple(1, 2, 3));
}

TEST(FunctionTest, TestCopyConstructedFromEmptyFunction)
{
    infra::Function<void()> f;
    infra::Function<void()> copy(f);
    EXPECT_FALSE(static_cast<bool>(copy));
}

TEST(FunctionTest, TestCopyConstruct)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    infra::Function<void()> f2(f);
    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f2();
}

TEST(FunctionTest, TestCopyConstructTwice)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    infra::Function<void()> f2(f);
    infra::Function<void()> f3(f2);
    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f3();
}

TEST(FunctionTest, TestCopyAssignFromEmptyFunction)
{
    infra::Function<void()> f;
    infra::Function<void()> copy;
    copy = f;
    EXPECT_FALSE(static_cast<bool>(copy));
}

TEST(FunctionTest, TestCopyAssign)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    infra::Function<void()> f2;
    f2 = f;
    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f2();
}

TEST(FunctionTest, TestAssignNullptr)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    f = nullptr;
    EXPECT_FALSE(static_cast<bool>(f));
}

TEST(FunctionTest, TestAssign)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f;
    f = [&m]() { m.callback(); };
    EXPECT_CALL(m, callback());
    f();
}

TEST(FunctionTest, TestSwapFilledEmpty)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    infra::Function<void()> f2;
    swap(f, f2);
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f2();
}

TEST(FunctionTest, TestSwapFilledFilled)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f([&m]() { m.callback(); });
    infra::Function<void()> f2(infra::emptyFunction);
    swap(f, f2);
    EXPECT_TRUE(static_cast<bool>(f));
    EXPECT_CALL(m, callback());
    f2();
}

TEST(FunctionTest, TestSwapEmptyFilled)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f;
    infra::Function<void()> f2([&m]() { m.callback(); });
    swap(f, f2);
    EXPECT_FALSE(static_cast<bool>(f2));
    EXPECT_CALL(m, callback());
    f();
}

TEST(FunctionTest, TestSwapEmptyEmpty)
{
    infra::MockCallback<void()> m;
    infra::Function<void()> f;
    infra::Function<void()> f2;
    swap(f, f2);
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_FALSE(static_cast<bool>(f2));
}

TEST(FunctionTest, TestReturnValue)
{
    infra::Function<int()> f([]() { return 1; });
    EXPECT_EQ(1, f());
}

TEST(FunctionTest, TestParameter)
{
    infra::MockCallback<void(int)> m;
    infra::Function<void(int)> f([&m](int x) { m.callback(x); });
    EXPECT_CALL(m, callback(1));
    f(1);
}

TEST(FunctionProxyTest, TestProxy)
{
    infra::MockCallback<void()> m;
    infra::Function<void(), 40> p([&m]() { m.callback(); });

    EXPECT_CALL(m, callback());
    infra::Function<void(), sizeof(void*)> f([&p]() { p(); });
    f();
}

TEST(FunctionTest, TestCompareToNullptr)
{
    infra::Function<void()> f;

    EXPECT_EQ(f, nullptr);
    EXPECT_EQ(nullptr, f);

    infra::Function<void()> g([]() {});

    EXPECT_NE(g, nullptr);
    EXPECT_NE(nullptr, g);
}

TEST(FunctionTest, Execute)
{
    infra::MockCallback<void()> m;
    EXPECT_CALL(m, callback());
    infra::Execute execute([&m]() { m.callback(); });
}

TEST(FunctionTest, ExecuteWithSize)
{
    infra::MockCallback<void()> m;
    EXPECT_CALL(m, callback());
    int a, b, c;
    infra::Execute::WithExtraSize<4 * sizeof(void*)> execute([&m, a, b, c]() { m.callback(); });
}

TEST(FunctionTest, ExecuteOnDestruction)
{
    infra::MockCallback<void()> m;

    {
        infra::ExecuteOnDestruction execute([&m]() { m.callback(); });

        EXPECT_CALL(m, callback());
    }
}

TEST(FunctionTest, ExecuteOnDestructionWithSize)
{
    infra::MockCallback<void()> m;
    int a, b, c;

    {
        infra::ExecuteOnDestruction::WithExtraSize<4 * sizeof(void*)> execute([&m, a, b, c]() { m.callback(); });

        EXPECT_CALL(m, callback());
    }
}
