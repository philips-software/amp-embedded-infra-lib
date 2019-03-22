#ifndef UPGRADE_VERIFIER_HPP
#define UPGRADE_VERIFIER_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"

namespace application
{
    class Verifier
    {
    public:
        virtual bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const = 0;

    protected:
        ~Verifier() = default;
    };
}

#endif
