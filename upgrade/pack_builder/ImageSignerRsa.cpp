#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "upgrade/pack_builder/ImageSignerRsa.hpp"

//TICS -CON#002

namespace application
{
    ImageSignerRsa::ImageSignerRsa(hal::SynchronousRandomDataGenerator& randomDataGenerator, const RsaPublicKey& publicKey, const RsaPrivateKey& privateKey)
        : publicKey(publicKey)
        , privateKey(privateKey)
        , randomDataGenerator(randomDataGenerator)
    {}

    uint16_t ImageSignerRsa::SignatureMethod() const
    {
        return signatureMethod;
    }

    uint16_t ImageSignerRsa::SignatureLength() const
    {
        return static_cast<uint16_t>(signature.size());
    }

    std::vector<uint8_t> ImageSignerRsa::ImageSignature(const std::vector<uint8_t>& image)
    {
        CalculateSha256(image);
        CalculateSignature(image);

        return std::vector<uint8_t>(signature.begin(), signature.end());
    }

    bool ImageSignerRsa::CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image)
    {
        std::copy(signature.begin(), signature.end(), this->signature.begin());
        CalculateSha256(image);

        mbedtls_rsa_context rsaCtx;
        mbedtls_rsa_init(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        rsaCtx.len = publicKey.N.size();
        rsaCtx.N.s = 1;
        rsaCtx.N.n = publicKey.N.size();
        rsaCtx.N.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKey.N.begin()));
        rsaCtx.E.s = 1;
        rsaCtx.E.n = publicKey.E.size();
        rsaCtx.E.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKey.E.begin()));
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

        if (mbedtls_rsa_check_pubkey(&rsaCtx) != 0)
            return false;

        bool success = mbedtls_rsa_rsassa_pss_verify(&rsaCtx, NULL, NULL, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, hash.size(), hash.data(), signature.data()) == 0
            && mbedtls_rsa_self_test(0) == 0;

        rsaCtx.N.p = nullptr;
        rsaCtx.E.p = nullptr;
        rsaCtx.D.p = nullptr;
        rsaCtx.P.p = nullptr;
        rsaCtx.Q.p = nullptr;
        rsaCtx.DP.p = nullptr;
        rsaCtx.DQ.p = nullptr;
        rsaCtx.QP.p = nullptr;
        mbedtls_rsa_free(&rsaCtx);

        return success;
    }

    void ImageSignerRsa::CalculateSha256(const std::vector<uint8_t>& image)
    {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, image.data(), image.size());
        mbedtls_sha256_finish(&ctx, hash.data());                                                                       //TICS !INT#030
        mbedtls_sha256_free(&ctx);
    }

    void ImageSignerRsa::CalculateSignature(const std::vector<uint8_t>& image)
    {
        int ret;

        mbedtls_rsa_context rsaCtx;
        mbedtls_rsa_init(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        rsaCtx.len = publicKey.N.size();
        rsaCtx.N.s = 1;
        rsaCtx.N.n = publicKey.N.size() / 4;
        rsaCtx.N.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKey.N.begin()));
        rsaCtx.E.s = 1;
        rsaCtx.E.n = publicKey.E.size() / 4;
        rsaCtx.E.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(publicKey.E.begin()));
        rsaCtx.D.s = 1;
        rsaCtx.D.n = privateKey.D.size() / 4;
        rsaCtx.D.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(privateKey.D.begin()));
        rsaCtx.P.s = 1;
        rsaCtx.P.n = privateKey.P.size() / 4;
        rsaCtx.P.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(privateKey.P.begin()));
        rsaCtx.Q.s = 1;
        rsaCtx.Q.n = privateKey.Q.size() / 4;
        rsaCtx.Q.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(privateKey.Q.begin()));
        rsaCtx.DP.s = 1;
        rsaCtx.DP.n = privateKey.DP.size() / 4;
        rsaCtx.DP.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(privateKey.DP.begin()));
        rsaCtx.DQ.s = 1;
        rsaCtx.DQ.n = privateKey.DQ.size() / 4;
        rsaCtx.DQ.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(privateKey.DQ.begin()));
        rsaCtx.QP.s = 1;
        rsaCtx.QP.n = privateKey.QP.size() / 4;
        rsaCtx.QP.p = reinterpret_cast<mbedtls_mpi_uint*>(const_cast<uint8_t*>(privateKey.QP.begin()));
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

        if (rsaCtx.len != signature.size())
            throw std::exception("Key length wrong");

        ret = mbedtls_rsa_check_privkey(&rsaCtx);
        if (ret != 0)
            throw std::exception("Public key is invalid");

        ret = mbedtls_rsa_rsassa_pss_sign(&rsaCtx, RandomNumberGenerator, this, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, hash.size(), hash.data(), signature.data());
        if (ret != 0)
            throw std::exception("Failed to calculate signature");

        rsaCtx.N.p = nullptr;
        rsaCtx.E.p = nullptr;
        rsaCtx.D.p = nullptr;
        rsaCtx.P.p = nullptr;
        rsaCtx.Q.p = nullptr;
        rsaCtx.DP.p = nullptr;
        rsaCtx.DQ.p = nullptr;
        rsaCtx.QP.p = nullptr;
        mbedtls_rsa_free(&rsaCtx);
    }

    int ImageSignerRsa::RandomNumberGenerator(void* rng_state, unsigned char* output, size_t len)
    {
        std::vector<uint8_t> data(len, 0);
        reinterpret_cast<ImageSignerRsa*>(rng_state)->randomDataGenerator.GenerateRandomData(data);
        std::copy(data.begin(), data.end(), output);
        return 0;
    }
}
