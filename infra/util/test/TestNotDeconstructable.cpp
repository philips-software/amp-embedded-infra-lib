
#include "infra/util/NonDestructable.hpp"
#include "gtest/gtest.h"

class Foo : public infra::NonDestructable
{};

TEST(NonDestructableTest, not_deconstructable)
{
    EXPECT_DEATH(Foo foo;, "");
}

TEST(NonDestructableTest, destructable_when_allowed)
{
    Foo foo;
    foo.AllowDestructionOfNonDestructable();
}
