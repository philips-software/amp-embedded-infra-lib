
#include "infra/util/NotDestructible.hpp"
#include "gtest/gtest.h"

namespace
{
    class Foo : public infra::NotDestructible
    {};
}

TEST(NotDestructibleTest, not_destructible)
{
    EXPECT_DEATH(Foo foo;, "");
}

TEST(NotDestructibleTest, destructible_when_allowed)
{
    Foo foo;
    foo.AllowDestructionOfNotDestructible();
}
