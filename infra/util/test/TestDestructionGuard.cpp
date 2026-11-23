
#include "infra/util/DestructionGuard.hpp"
#include "gtest/gtest.h"

namespace
{
    class Foo : public infra::DestructionGuard
    {};
}

#if defined(EMIL_ENABLE_DESTRUCTION_GUARD) && !defined(EMIL_MUTATION_TESTING)
TEST(DestructionGuardTest, not_destructible_when_not_allowed)
{
    EXPECT_DEATH(Foo foo;, "");
}
#endif

#ifndef EMIL_ENABLE_DESTRUCTION_GUARD
TEST(DestructionGuardTest, always_destructible_when_not_enabled)
{
    Foo foo;
}
#endif

TEST(DestructionGuardTest, destructible_when_allowed)
{
    Foo foo;
    foo.MarkAsSafeToDestruct();
}
