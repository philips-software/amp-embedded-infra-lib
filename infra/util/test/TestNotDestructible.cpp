
#include "infra/util/NonDestructible.hpp"
#include "gtest/gtest.h"

namespace
{
    class Foo : public infra::NonDestructible
    {};
}

TEST(NonDestructibleTest, not_destructible)
{
    EXPECT_DEATH(Foo foo;, "");
}

TEST(NonDestructibleTest, destructible_when_allowed)
{
    Foo foo;
    foo.AllowDestructionOfNonDestructible();
}
