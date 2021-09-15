#include "upgrade/pack_builder/ImageSignerEcDsa.hpp"
#include "crypto/micro-ecc/uECC.h"
#include "mbedtls/sha256.h"

namespace application
{
    hal::SynchronousRandomDataGenerator* ImageSignerEcDsa::randomDataGenerator = nullptr;

    ImageSignerEcDsa::ImageSignerEcDsa(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey)
        : publicKey(publicKey)
        , privateKey(privateKey)
    {
        ImageSignerEcDsa::randomDataGenerator = &randomDataGenerator;
        uECC_set_rng(RandomNumberGenerator);
    }

    uint16_t ImageSignerEcDsa::SignatureMethod() const
    {
        return signatureMethod;
    }

    uint16_t ImageSignerEcDsa::SignatureLength() const
    {
        return static_cast<uint16_t>(signature.size());
    }

    std::vector<uint8_t> ImageSignerEcDsa::ImageSignature(const std::vector<uint8_t>& image)
    {
        CalculateSha256(image);
        CalculateSignature(image);

        return std::vector<uint8_t>(signature.begin(), signature.end());
    }

    bool ImageSignerEcDsa::CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image)
    {
        std::copy(signature.begin(), signature.end(), this->signature.begin());
        CalculateSha256(image);

        if (publicKey.size() != signature.size())
            return false;

        return uECC_verify(publicKey.begin(), hash.data(), hash.size(), signature.data(), uECC_secp224r1()) == 1;
    }

    void ImageSignerEcDsa::CalculateSha256(const std::vector<uint8_t>& image)
    {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, image.data(), image.size());
        mbedtls_sha256_finish(&ctx, hash.data());
        mbedtls_sha256_free(&ctx);
    }

    void ImageSignerEcDsa::CalculateSignature(const std::vector<uint8_t>& image)
    {
        if (privateKey.size() != signature.size())
            throw std::runtime_error("Key length wrong");

        std::array<uint8_t, keyLength / 8 * 2> publicKeyForVerification;
        int result = uECC_compute_public_key(privateKey.begin(), publicKeyForVerification.data(), uECC_secp224r1());
        if (result != 1)
            throw std::runtime_error("Failed to compute public key");

        if (!std::equal(publicKey.begin(), publicKey.end(), publicKeyForVerification.begin()))
            throw std::runtime_error("Inconsistent private key pair");

        int ret = uECC_sign(privateKey.begin(), hash.data(), hash.size(), signature.data(), uECC_secp224r1());
        if (ret != 1)
            throw std::runtime_error("Failed to calculate signature");
    }

    int ImageSignerEcDsa::RandomNumberGenerator(uint8_t* dest, unsigned size)
    {
        std::vector<uint8_t> entropy(size, 0);
        randomDataGenerator->GenerateRandomData(entropy);
        std::copy(entropy.begin(), entropy.end(), dest);
        return 1;
    }
}
