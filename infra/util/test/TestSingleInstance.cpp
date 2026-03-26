
#include "infra/util/SingleInstance.hpp"
#include "gtest/gtest.h"

class Foo : public infra::SingleInstance<Foo>
{};

class Bar : public infra::SingleInstance<Bar>
{};

TEST(SingleInstanceTest, once_instance_allowed)
{
    {
        Foo foo;
    }
}

TEST(SingleInstanceTest, after_destruction_new_instance_allowed)
{
    {
        Foo foo1;
    }
    Foo foo2;
}

TEST(SingleInstanceTest, multiple_tags_allowed)
{
    Foo foo;
    Bar bar;
}

TEST(SingleInstanceTest, two_instances_not_allowed)
{
    Foo foo1;

    EXPECT_DEATH(Foo foo2;, "");
}
