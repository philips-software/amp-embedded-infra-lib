#ifndef INFRA_UTIL_NOT_DESTRUCTIBLE_HPP
#define INFRA_UTIL_NOT_DESTRUCTIBLE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    class DestructionGuard
    {
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
    private:
        bool canSafelyDestruct = false;
    };
}

#endif
