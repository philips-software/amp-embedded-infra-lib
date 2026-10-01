#ifndef INFRA_UTIL_NOT_DESTRUCTIBLE_HPP
#define INFRA_UTIL_NOT_DESTRUCTIBLE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    class DestructionGuard
    {
        bool canSafelyDestruct = false;

    public:
        ~DestructionGuard()
        {
            really_assert_with_msg(canSafelyDestruct, "Destruction not allowed");
        }

#ifdef EMIL_HOST_BUILD
        void AllowDestructionOfNotDestructible()
        {
            canSafelyDestruct = true;
        }
#endif
    };
}

#endif // INFRA_UTIL_NOT_DESTRUCTIBLE_HPP
