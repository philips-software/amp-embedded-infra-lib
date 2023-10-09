#include "upgrade/boot_loader/VerifierEcDsa.hpp"
#include "mbedtls/sha256.h"
#include "uECC.h"

namespace application
{
    VerifierEcDsa::VerifierEcDsa(infra::ConstByteRange key)
        : key(key)
    {
        assert(key.size() == 56);
    }

    bool VerifierEcDsa::IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const
    {
        std::array<uint8_t, 32> messageHash = Hash(flash, data);
        std::array<uint8_t, 56> storedSignature;

        if (signature.second - signature.first != storedSignature.size())
            return false;

        flash.ReadBuffer(storedSignature, signature.first);

        return uECC_verify(key.begin(),
                   messageHash.data(),
                   messageHash.size(),
                   storedSignature.data(),
                   uECC_secp224r1()) == 1;
    }
}
