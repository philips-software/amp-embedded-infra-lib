#include "upgrade/pack_builder/ImageSignerRsa.hpp"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

//TICS -CON#002

namespace
{
    // System Workbench's GCC does not yet support std::make_reverse_iterator
    namespace polyfill
    {
        template<class Iterator>
        std::reverse_iterator<Iterator> make_reverse_iterator(Iterator i)
        {
            return std::reverse_iterator<Iterator>(i);
        }
    }

    std::vector<uint8_t> SwapBytes(infra::ConstByteRange range)
    {
        std::vector<uint8_t> swapped(range.size());

        std::copy(range.begin(), range.end(), polyfill::make_reverse_iterator(swapped.end()));

        return swapped;
    }
}

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
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        mbedtls_rsa_import_raw(&rsaCtx,
            SwapBytes(publicKey.N).data(), publicKey.N.size(),
            nullptr, 0,
            nullptr, 0,
            nullptr, 0,
            SwapBytes(publicKey.E).data(), publicKey.E.size());

        if (mbedtls_rsa_complete(&rsaCtx) != 0)
            return false;

        if (mbedtls_rsa_check_pubkey(&rsaCtx) != 0)
            return false;

        bool success = mbedtls_rsa_rsassa_pss_verify(&rsaCtx, NULL, NULL, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, hash.size(), hash.data(), signature.data()) == 0 && mbedtls_rsa_self_test(0) == 0;

        mbedtls_rsa_free(&rsaCtx);

        return success;
    }

    void ImageSignerRsa::CalculateSha256(const std::vector<uint8_t>& image)
    {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, image.data(), image.size());
        mbedtls_sha256_finish(&ctx, hash.data());
        mbedtls_sha256_free(&ctx);
    }

    void ImageSignerRsa::CalculateSignature(const std::vector<uint8_t>& image)
    {
        int ret;

        mbedtls_rsa_context rsaCtx;
        mbedtls_rsa_init(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        mbedtls_rsa_set_padding(&rsaCtx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

        mbedtls_rsa_import_raw(&rsaCtx,
            SwapBytes(publicKey.N).data(), publicKey.N.size(),
            SwapBytes(privateKey.P).data(), privateKey.P.size(),
            SwapBytes(privateKey.Q).data(), privateKey.Q.size(),
            SwapBytes(privateKey.D).data(), privateKey.D.size(),
            SwapBytes(publicKey.E).data(), publicKey.E.size());

        if (mbedtls_rsa_complete(&rsaCtx) != 0)
            throw std::runtime_error("Invalid public or private key parameters");

        if (rsaCtx.len != signature.size())
            throw std::runtime_error("Key length wrong");

        ret = mbedtls_rsa_check_privkey(&rsaCtx);
        if (ret != 0)
            throw std::runtime_error("Public or private key is invalid");

        ret = mbedtls_rsa_rsassa_pss_sign(&rsaCtx, RandomNumberGenerator, this, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, hash.size(), hash.data(), signature.data());
        if (ret != 0)
            throw std::runtime_error("Failed to calculate signature");

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
