#ifndef UPGRADE_VERIFIER_EC_DSA_HPP
#define UPGRADE_VERIFIER_EC_DSA_HPP

#include "upgrade/boot_loader/VerifierHashOnly.hpp"

namespace application
{
    class VerifierEcDsa
        : public VerifierHashOnly
    {
    public:
        explicit VerifierEcDsa(infra::ConstByteRange key);

        bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;

    private:
        infra::ConstByteRange key;
    };
}

#endif
