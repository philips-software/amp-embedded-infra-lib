#ifndef INFRA_UTIL_DESTRUCTION_GUARD_HPP
#define INFRA_UTIL_DESTRUCTION_GUARD_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    class DestructionGuard
    {
    protected:
        ~DestructionGuard()
        {
#ifdef EMIL_ENABLE_DESTRUCTION_GUARD
            really_assert_with_msg(canSafelyDestruct, "Destruction not allowed");
#endif
        }

    public:
        void MarkAsSafeToDestruct()
        {
            canSafelyDestruct = true;
        }

    private:
        bool canSafelyDestruct = false;
    };
}

#endif
