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
        auto storedSignature = GetSignatureStorage(signature);

        if (storedSignature.empty())
            return false;

        flash.ReadBuffer(storedSignature, signature.first);
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

    infra::ByteRange VerifierEcDsa224::GetSignatureStorage(const hal::SynchronousFlash::Range& signature) const
    {
        if (signature.second - signature.first != signatureStorage.size())
            return infra::ByteRange();
        else
            return infra::MakeRange(signatureStorage);
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

    infra::ByteRange VerifierEcDsa256::GetSignatureStorage(const hal::SynchronousFlash::Range& signature) const
    {
        if (signature.second - signature.first != signatureStorage.size())
            return infra::ByteRange();
        else
            return infra::MakeRange(signatureStorage);
    }

    uECC_Curve VerifierEcDsa256::GetCurve() const
    {
        return uECC_secp256r1();
    }
}
