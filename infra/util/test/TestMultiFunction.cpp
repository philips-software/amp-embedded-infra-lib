#include "infra/util/MultiFunction.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gtest/gtest.h"

TEST(MultiFunctionTest, construct_empty)
{
    infra::MultiFunction<void()> f;
    EXPECT_FALSE(f);
    EXPECT_TRUE(!f);
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_TRUE(f == nullptr);
    EXPECT_TRUE(nullptr == f);
    EXPECT_FALSE(f != nullptr);
    EXPECT_FALSE(nullptr != f);
}

TEST(MultiFunctionTest, construct_empty_with_nullptr)
{
    infra::MultiFunction<void()> f(nullptr);
    EXPECT_FALSE(f);
}

TEST(MultiFunctionTest, construct_with_function)
{
    infra::VerifyingFunction<void()> m;

    infra::MultiFunction<void()> f([&]()
        {
            m.callback();
        });
    f();
}

TEST(MultiFunctionTest, multi_function_construct_with_first)
{
    infra::VerifyingFunction<void()> m;

    infra::MultiFunction<void()>::And<void(bool)>::And<void(int)> f([&]()
        {
            m.callback();
        });
    f();
}

TEST(MultiFunctionTest, multi_function_construct_with_second)
{
    infra::VerifyingFunction<void(bool)> m(true);

    infra::MultiFunction<void()>::And<void(bool)> f([&](bool v)
        {
            m.callback(v);
        });
    f(true);
}

TEST(MultiFunctionTest, reassign_multi_function)
{
    infra::VerifyingFunction<void(bool)> m(true);

    infra::MultiFunction<void()>::And<void(bool)> f([]() {});
    f = [&](bool v)
    {
        m.callback(v);
    };
    f(true);
}

TEST(MultiFunctionTest, assign_nullptr)
{
    infra::MultiFunction<void()>::And<void(bool)> f([]() {});
    f = nullptr;
    EXPECT_FALSE(f);
}

TEST(MultiFunctionTest, return_value)
{
    infra::MockCallback<int()> m;
    EXPECT_CALL(m, callback()).WillOnce(testing::Return(5));

    infra::MultiFunction<int()>::And<void(bool)> f([&]()
        {
            return m.callback();
        });
    EXPECT_EQ(5, f());
}

TEST(MultiFunctionTest, multi_function_Invocable)
{
    infra::MultiFunction<void()>::And<void(bool)> f([&](bool v) {});
    EXPECT_TRUE(f.Invocable(true));
    EXPECT_FALSE(f.Invocable());
}

TEST(MultiFunctionTest, mutable_function)
{
    infra::MultiFunction<int()> f([n = 0]() mutable
        {
            return ++n;
        });
    EXPECT_EQ(1, f());
    EXPECT_EQ(2, f());
    EXPECT_EQ(3, f());
}
