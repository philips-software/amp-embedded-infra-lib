
#include "infra/util/DestructionGuard.hpp"
#include "gtest/gtest.h"

namespace
{
    class Foo : public infra::DestructionGuard
    {};
}

TEST(DestructionGuardTest, not_destructible)
{
    EXPECT_DEATH(Foo foo;, "");
}

TEST(DestructionGuardTest, destructible_when_allowed)
{
    Foo foo;
    foo.AllowDestructionOfNotDestructible();
}
