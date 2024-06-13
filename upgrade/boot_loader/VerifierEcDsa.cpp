#include "upgrade/boot_loader/VerifierEcDsa.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/MemoryRange.hpp"
#include "mbedtls/sha256.h"
#include "uECC.h"

namespace application
{
    VerifierEcDsa::VerifierEcDsa(infra::ConstByteRange key)
        : key(key)
    {}

    bool VerifierEcDsa::IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const
    {
        std::array<uint8_t, 32> messageHash = Hash(flash, data);
        auto storedSignature = GetSignatureStorage(flash, signature);

        if (storedSignature.size() == 0)
            return false;

        return uECC_verify(key.begin(),
                   messageHash.data(),
                   messageHash.size(),
                   storedSignature.begin(),
                   GetCurve()) == 1;
    }

    VerifierEcDsa224::VerifierEcDsa224(infra::ConstByteRange key)
        : VerifierEcDsa(key)
    {
        assert(key.size() == signSize);
    }

    infra::ConstByteRange VerifierEcDsa224::GetSignatureStorage(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature) const
    {
        if (signature.second - signature.first != signatureStorage.size())
            return infra::ConstByteRange();

        flash.ReadBuffer(infra::ConstCastByteRange(signatureStorage), signature.first);
        return signatureStorage;
    }

    uECC_Curve VerifierEcDsa224::GetCurve() const
    {
        return uECC_secp224r1();
    }

    VerifierEcDsa256::VerifierEcDsa256(infra::ConstByteRange key)
        : VerifierEcDsa(key)
    {
        assert(key.size() == signSize);
    }

    infra::ConstByteRange VerifierEcDsa256::GetSignatureStorage(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature) const
    {
        if (signature.second - signature.first != signatureStorage.size())
            return infra::ConstByteRange();

        flash.ReadBuffer(infra::ConstCastByteRange(signatureStorage), signature.first);
        return signatureStorage;
    }

    uECC_Curve VerifierEcDsa256::GetCurve() const
    {
        return uECC_secp256r1();
    }
}
