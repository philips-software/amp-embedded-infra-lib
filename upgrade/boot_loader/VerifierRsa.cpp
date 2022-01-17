#include "mbedtls/sha256.h"
#include "mbedtls/rsa.h"
#include "crypto/micro-ecc/uECC.h"
#include "upgrade/boot_loader/VerifierRsa.hpp"

namespace application
{
    VerifierRsa::VerifierRsa(infra::ConstByteRange publicKeyN, infra::ConstByteRange publicKeyE)
        : publicKeyN(publicKeyN)
        , publicKeyE(publicKeyE)
    {}

    bool VerifierRsa::IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const
    {
        std::array<uint8_t, 32> messageHash;

        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);

        uint32_t message = data.first;
        while (message != data.second)
        {
            std::array<uint8_t, 256> buffer;
            std::size_t size = std::min(buffer.size(), static_cast<std::size_t>(data.second - message));
            flash.ReadBuffer(infra::ByteRange(buffer.data(), buffer.data() + size), message);
            mbedtls_sha256_update(&ctx, buffer.data(), size);
            message += size;
        }

        mbedtls_sha256_finish(&ctx, messageHash.data());
        mbedtls_sha256_free(&ctx);

        std::array<uint8_t, 56> theSignature;
        if (signature.second - signature.first != theSignature.size())
            return false;
        flash.ReadBuffer(theSignature, signature.first);

        mbedtls_rsa_context rsaCtx;
        mbedtls_rsa_init(&rsaCtx);
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        rsaCtx.MBEDTLS_PRIVATE(len) = publicKeyN.size();
        rsaCtx.MBEDTLS_PRIVATE(N).MBEDTLS_PRIVATE(s) = 1;
        rsaCtx.MBEDTLS_PRIVATE(N).MBEDTLS_PRIVATE(n) = publicKeyN.size();
        rsaCtx.MBEDTLS_PRIVATE(N).MBEDTLS_PRIVATE(p) = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKeyN.begin()));
        rsaCtx.MBEDTLS_PRIVATE(E).MBEDTLS_PRIVATE(s) = 1;
        rsaCtx.MBEDTLS_PRIVATE(E).MBEDTLS_PRIVATE(n) = publicKeyE.size();
        rsaCtx.MBEDTLS_PRIVATE(E).MBEDTLS_PRIVATE(p) = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKeyE.begin()));
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

        int ret = mbedtls_rsa_rsassa_pss_verify(&rsaCtx, MBEDTLS_MD_SHA256, messageHash.size(), messageHash.data(), theSignature.data());

        rsaCtx.MBEDTLS_PRIVATE(N).MBEDTLS_PRIVATE(p) = nullptr;
        rsaCtx.MBEDTLS_PRIVATE(E).MBEDTLS_PRIVATE(p) = nullptr;
        mbedtls_rsa_free(&rsaCtx);

        return ret == 0;
    }
}
