#ifndef UPGRADE_VERIFIER_NONE_HPP
#define UPGRADE_VERIFIER_NONE_HPP

#include "upgrade/boot_loader/Verifier.hpp"

namespace application
{
    class VerifierNone
        : public Verifier
    {
    public:
        virtual bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;
    };
}

#endif
