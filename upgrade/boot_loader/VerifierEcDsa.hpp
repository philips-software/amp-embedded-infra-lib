#ifndef UPGRADE_VERIFIER_EC_DSA_HPP
#define UPGRADE_VERIFIER_EC_DSA_HPP

#include "upgrade/boot_loader/Verifier.hpp"

namespace application
{
    class VerifierEcDsa
        : public Verifier
    {
    public:
        explicit VerifierEcDsa(infra::ConstByteRange key);

        virtual bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;

    private:
        infra::ConstByteRange key;
    };
}

#endif
