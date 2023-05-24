#ifndef UPGRADE_VERIFIER_HASH_ONLY_HPP
#define UPGRADE_VERIFIER_HASH_ONLY_HPP

#include "upgrade/boot_loader/Verifier.hpp"

namespace application
{
    class VerifierHashOnly
        : public Verifier
    {
    public:
        bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;

    protected:
        std::array<uint8_t, 32> Hash(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& data) const;
    };
}

#endif
