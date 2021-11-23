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
        mbedtls_rsa_init(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        rsaCtx.len = publicKeyN.size();
        rsaCtx.N.s = 1;
        rsaCtx.N.n = publicKeyN.size();
        rsaCtx.N.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKeyN.begin()));
        rsaCtx.E.s = 1;
        rsaCtx.E.n = publicKeyE.size();
        rsaCtx.E.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKeyE.begin()));
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

        int ret = mbedtls_rsa_rsassa_pss_verify(&rsaCtx, nullptr, nullptr, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, messageHash.size(), messageHash.data(), theSignature.data());

        rsaCtx.N.p = nullptr;
        rsaCtx.E.p = nullptr;
        mbedtls_rsa_free(&rsaCtx);

        return ret == 0;
    }
}
