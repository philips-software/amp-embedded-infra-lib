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
#ifdef EMIL_ENABLE_DESTRUCTION_GUARD
            really_assert_with_msg(canSafelyDestruct, "Destruction not allowed");
#endif
        }

        void MarkAsSafeToDeconstruct()
        {
            canSafelyDestruct = true;
        }

    private:
        bool canSafelyDestruct = false;
    };
}

#endif
