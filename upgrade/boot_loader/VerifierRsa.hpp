#ifndef UPGRADE_VERIFIER_RSA_HPP
#define UPGRADE_VERIFIER_RSA_HPP

#include "upgrade/boot_loader/Verifier.hpp"

namespace application
{
    class VerifierRsa
        : public Verifier
    {
    public:
        VerifierRsa(infra::ConstByteRange publicKeyN, infra::ConstByteRange publicKeyE);

        virtual bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;

    private:
        infra::ConstByteRange publicKeyN;
        infra::ConstByteRange publicKeyE;
    };
}

#endif
