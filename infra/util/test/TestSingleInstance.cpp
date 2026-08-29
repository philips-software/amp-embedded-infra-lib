
#include "infra/util/SingleInstance.hpp"
#include "gtest/gtest.h"
#include <type_traits>

namespace
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    class Bar : public infra::SingleInstance<Bar>
    {};

    class SingleInstanceTest
        : public testing::Test
    {
    public:
        ~SingleInstanceTest()
        {
            Foo::ResetSingleInstanceCounter();
            Bar::ResetSingleInstanceCounter();
        }
    };
}

TEST_F(SingleInstanceTest, once_instance_allowed)
{
    Foo foo;
}

TEST_F(SingleInstanceTest, after_destruction_new_instance_allowed)
{
    {
        Foo foo1;
    }
    Foo foo2;
}

TEST_F(SingleInstanceTest, multiple_tags_allowed)
{
    Foo foo;
    Bar bar;
}

TEST_F(SingleInstanceTest, two_instances_not_allowed)
{
    Foo foo1;

    EXPECT_DEATH(Foo foo2;, "");
}

TEST_F(SingleInstanceTest, is_not_copy_constructible)
{
    static_assert(!std::is_copy_constructible_v<Foo>);
}

TEST_F(SingleInstanceTest, is_not_move_constructible)
{
    static_assert(!std::is_move_constructible_v<Foo>);
}

TEST_F(SingleInstanceTest, is_not_copy_assignable)
{
    static_assert(!std::is_copy_assignable_v<Foo>);
}

TEST_F(SingleInstanceTest, is_not_move_assignable)
{
    static_assert(!std::is_move_assignable_v<Foo>);
}
