#include "mbedtls/sha256.h"
#include "crypto/micro-ecc/uECC.h"
#include "upgrade/boot_loader/VerifierEcDsa.hpp"

namespace application
{
    VerifierEcDsa::VerifierEcDsa(infra::ConstByteRange key)
        : key(key)
    {}

    bool VerifierEcDsa::IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const
    {
        std::array<uint8_t, 32> messageHash;

        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);

        uint32_t message = data.first;
        while (message != data.second)
        {
            std::array<uint8_t, 256> buffer;
            std::size_t size = std::min<std::size_t>(buffer.size(), data.second - message);
            flash.ReadBuffer(infra::ByteRange(buffer.data(), buffer.data() + size), message);
            mbedtls_sha256_update(&ctx, buffer.data(), size);
            message += size;
        }

        mbedtls_sha256_finish(&ctx, messageHash.data());
        mbedtls_sha256_free(&ctx);

        assert(key.size() == 56);
        std::array<uint8_t, 56> theSignature;
        if (signature.second - signature.first != theSignature.size())
            return false;
        flash.ReadBuffer(theSignature, signature.first);

        return uECC_verify(key.begin(),
            messageHash.data(),
            messageHash.size(),
            theSignature.data(),
            uECC_secp224r1()) == 1;
    }
}
