
#include "infra/util/DestructionGuard.hpp"
#include "gtest/gtest.h"

#if defined(EXPECT_EMIL_DESTRUCTION_GUARD_DISABLED) && defined(EMIL_ENABLE_DESTRUCTION_GUARD)
#error EMIL_ENABLE_DESTRUCTION_GUARD must be disabled for this test target
#elif !defined(EXPECT_EMIL_DESTRUCTION_GUARD_DISABLED) && !defined(EMIL_ENABLE_DESTRUCTION_GUARD)
#error EMIL_ENABLE_DESTRUCTION_GUARD must be enabled for this test target
#endif

namespace
{
    class Foo : public infra::DestructionGuard
    {};
}

#if defined(EMIL_ENABLE_DESTRUCTION_GUARD) && defined(EMIL_MUTATION_TESTING)
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
