#ifndef INFRA_UTIL_NON_DESTRUCTABLE_HPP
#define INFRA_UTIL_NON_DESTRUCTABLE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    class NonDestructable
    {
        bool canSafetyDeconstruct = false;

    public:
        ~NonDestructable()
        {
            really_assert_with_msg(canSafetyDeconstruct, "Destruction not allowed");
        }

#ifdef EMIL_HOST_BUILD
        void AllowDestructionOfNonDestructable()
        {
            canSafetyDeconstruct = true;
        }
#endif
    };
}

#endif // INFRA_UTIL_NON_DESTRUCTABLE_HPP
