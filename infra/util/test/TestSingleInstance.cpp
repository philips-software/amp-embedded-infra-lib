
#include "infra/util/SingleInstance.hpp"
#include "gtest/gtest.h"
#include <type_traits>

TEST(SingleInstanceTest, once_instance_allowed)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    Foo foo;
}

TEST(SingleInstanceTest, after_destruction_new_instance_allowed)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    {
        Foo foo1;
    }
    Foo foo2;
}

TEST(SingleInstanceTest, multiple_tags_allowed)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    class Bar : public infra::SingleInstance<Bar>
    {};

    Foo foo;
    Bar bar;
}

#ifndef EMIL_MUTATION_TESTING
TEST(SingleInstanceTest, two_instances_not_allowed)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    Foo foo1;

    EXPECT_DEATH(Foo foo2;, "");
}
#endif

TEST(SingleInstanceTest, is_not_copy_constructible)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    static_assert(!std::is_copy_constructible_v<Foo>);
}

TEST(SingleInstanceTest, is_not_move_constructible)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    static_assert(!std::is_move_constructible_v<Foo>);
}

TEST(SingleInstanceTest, is_not_copy_assignable)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    static_assert(!std::is_copy_assignable_v<Foo>);
}

TEST(SingleInstanceTest, is_not_move_assignable)
{
    class Foo : public infra::SingleInstance<Foo>
    {};

    static_assert(!std::is_move_assignable_v<Foo>);
}
