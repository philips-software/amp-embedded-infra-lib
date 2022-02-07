#ifndef UPGRADE_VERIFIER_HASH_ONLY_HPP
#define UPGRADE_VERIFIER_HASH_ONLY_HPP

#include "upgrade/boot_loader/Verifier.hpp"

namespace application
{
    class VerifierHashOnly
        : public Verifier
    {
    public:
        virtual bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;
    };
}

#endif
