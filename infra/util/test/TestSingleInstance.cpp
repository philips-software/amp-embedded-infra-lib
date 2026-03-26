
#include "infra/util/SingleInstance.hpp"
#include "gtest/gtest.h"

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
