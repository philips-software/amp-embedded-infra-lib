#ifndef UPGRADE_IMAGE_SIGNER_EC_DSA_HPP
#define UPGRADE_IMAGE_SIGNER_EC_DSA_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ByteRange.hpp"
#include "mbedtls/sha256.h"
#include "uECC.h"
#include "upgrade/pack_builder/ImageSigner.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace application
{
    template<size_t KEY_LENGTH>
    class ImageSignerEcDsa
        : public ImageSigner
    {
    public:
        ImageSignerEcDsa(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);

        uint16_t SignatureMethod() const override;
        uint16_t SignatureLength() const override;
        std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) override;
        bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

        static const uint16_t signatureMethod = 1;

    private:
        void CalculateSha256(const std::vector<uint8_t>& image);
        void CalculateSignature(const std::vector<uint8_t>& image);

        static int RandomNumberGenerator(uint8_t* dest, unsigned size);

        infra::ConstByteRange publicKey;
        infra::ConstByteRange privateKey;
        static hal::SynchronousRandomDataGenerator* randomDataGenerator;
        std::array<uint8_t, 32> hash;
        const size_t keyLength = KEY_LENGTH;
        std::array<uint8_t, KEY_LENGTH / 8 * 2> signature;
    };

    using ImageSignerEcDsa224 = ImageSignerEcDsa<224>;
    using ImageSignerEcDsa256 = ImageSignerEcDsa<256>;

    template<size_t KEY_LENGTH>
    hal::SynchronousRandomDataGenerator* ImageSignerEcDsa<KEY_LENGTH>::randomDataGenerator = nullptr;

    template<size_t KEY_LENGTH>
    ImageSignerEcDsa<KEY_LENGTH>::ImageSignerEcDsa(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey)
        : publicKey(publicKey)
        , privateKey(privateKey)
    {
        ImageSignerEcDsa<KEY_LENGTH>::randomDataGenerator = &randomDataGenerator;
        uECC_set_rng(RandomNumberGenerator);
    }

    template<size_t KEY_LENGTH>
    uint16_t ImageSignerEcDsa<KEY_LENGTH>::SignatureMethod() const
    {
        return signatureMethod;
    }

    template<size_t KEY_LENGTH>
    uint16_t ImageSignerEcDsa<KEY_LENGTH>::SignatureLength() const
    {
        return static_cast<uint16_t>(signature.size());
    }

    template<size_t KEY_LENGTH>
    std::vector<uint8_t> ImageSignerEcDsa<KEY_LENGTH>::ImageSignature(const std::vector<uint8_t>& image)
    {
        CalculateSha256(image);
        CalculateSignature(image);

        return std::vector<uint8_t>(signature.begin(), signature.end());
    }

    template<size_t KEY_LENGTH>
    bool ImageSignerEcDsa<KEY_LENGTH>::CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image)
    {
        std::copy(signature.begin(), signature.end(), this->signature.begin());
        CalculateSha256(image);

        if (publicKey.size() != signature.size())
            return false;

        return uECC_verify(publicKey.begin(), hash.data(), hash.size(), signature.data(), uECC_secp224r1()) == 1;
    }

    template<size_t KEY_LENGTH>
    void ImageSignerEcDsa<KEY_LENGTH>::CalculateSha256(const std::vector<uint8_t>& image)
    {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, image.data(), image.size());
        mbedtls_sha256_finish(&ctx, hash.data());
        mbedtls_sha256_free(&ctx);
    }

    template<size_t KEY_LENGTH>
    void ImageSignerEcDsa<KEY_LENGTH>::CalculateSignature(const std::vector<uint8_t>& image)
    {
        if (privateKey.size() != signature.size())
            throw std::runtime_error("Key length wrong");

        std::array<uint8_t, KEY_LENGTH / 8 * 2> publicKeyForVerification;
        int result = uECC_compute_public_key(privateKey.begin(), publicKeyForVerification.data(), uECC_secp224r1());
        if (result != 1)
            throw std::runtime_error("Failed to compute public key");

        if (!std::equal(publicKey.begin(), publicKey.end(), publicKeyForVerification.begin()))
            throw std::runtime_error("Inconsistent private key pair");

        int ret = uECC_sign(privateKey.begin(), hash.data(), hash.size(), signature.data(), uECC_secp224r1());
        if (ret != 1)
            throw std::runtime_error("Failed to calculate signature");
    }

    template<size_t KEY_LENGTH>
    int ImageSignerEcDsa<KEY_LENGTH>::RandomNumberGenerator(uint8_t* dest, unsigned size)
    {
        std::vector<uint8_t> entropy(size, 0);
        ImageSignerEcDsa<KEY_LENGTH>::randomDataGenerator->GenerateRandomData(entropy);
        std::copy(entropy.begin(), entropy.end(), dest);
        return 1;
    }
}

#endif
