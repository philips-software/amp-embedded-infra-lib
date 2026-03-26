#ifndef INFRA_UTIL_NOt_DESTRUCTIBLE_HPP
#define INFRA_UTIL_NOt_DESTRUCTIBLE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    class NotDestructible
    {
        bool canSafetyDeconstruct = false;

    public:
        ~NotDestructible()
        {
            really_assert_with_msg(canSafetyDeconstruct, "Destruction not allowed");
        }

#ifdef EMIL_HOST_BUILD
        void AllowDestructionOfNonDestructible()
        {
            canSafetyDeconstruct = true;
        }
#endif
    };
}

#endif // INFRA_UTIL_NOt_DESTRUCTIBLE_HPP
