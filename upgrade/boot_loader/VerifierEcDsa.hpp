#ifndef UPGRADE_VERIFIER_EC_DSA_HPP
#define UPGRADE_VERIFIER_EC_DSA_HPP

#include "infra/util/ByteRange.hpp"
#include "uECC.h"
#include "upgrade/boot_loader/VerifierHashOnly.hpp"

namespace application
{
    class VerifierEcDsa
        : public VerifierHashOnly
    {
    public:
        explicit VerifierEcDsa(infra::ConstByteRange key);
        virtual ~VerifierEcDsa() = default;

        bool IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override;

    private:
        virtual infra::ConstByteRange GetSignatureStorage(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature) const = 0;
        virtual uECC_Curve GetCurve() const = 0;

        infra::ConstByteRange key;
    };

    class VerifierEcDsa224
        : public VerifierEcDsa
    {
    public:
        explicit VerifierEcDsa224(infra::ConstByteRange key);

    private:
        infra::ConstByteRange GetSignatureStorage(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature) const override;
        uECC_Curve GetCurve() const override;

        static constexpr size_t signSize = 224 / 8 * 2;
        std::array<uint8_t, signSize> signatureStorage;
    };

    class VerifierEcDsa256
        : public VerifierEcDsa
    {
    public:
        explicit VerifierEcDsa256(infra::ConstByteRange key);

    private:
        infra::ConstByteRange GetSignatureStorage(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature) const override;
        uECC_Curve GetCurve() const override;

        static constexpr size_t signSize = 256 / 8 * 2;
        std::array<uint8_t, signSize> signatureStorage;
    };
}

#endif
