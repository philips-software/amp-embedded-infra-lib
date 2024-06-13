#ifndef UPGRADE_IMAGE_SIGNER_EC_DSA_HPP
#define UPGRADE_IMAGE_SIGNER_EC_DSA_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ByteRange.hpp"
#include "uECC.h"
#include "upgrade/pack_builder/ImageSigner.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace application
{
    class ImageSignerEcDsa
        : public ImageSigner
    {
    public:
        ImageSignerEcDsa(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);
        virtual ~ImageSignerEcDsa() = default;

        std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) override;
        bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

    private:
        virtual uECC_Curve GetCurve() const = 0;

        void CalculateSha256(const std::vector<uint8_t>& image);
        void CalculateSignature();
        static int RandomNumberGenerator(uint8_t* dest, unsigned size);
        static hal::SynchronousRandomDataGenerator* randomDataGenerator;

        infra::ConstByteRange publicKey;
        infra::ConstByteRange privateKey;
        std::array<uint8_t, 32> hash;
        std::vector<uint8_t> signature;
    };

    class ImageSignerEcDsa224
        : public ImageSignerEcDsa
    {
    public:
        ImageSignerEcDsa224(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);

        uint16_t SignatureMethod() const override;
        uint16_t SignatureLength() const override;

    private:
        uECC_Curve GetCurve() const override;

        static const uint16_t signatureMethod = 1;
        static const size_t keyLength = 224;
    };

    class ImageSignerEcDsa256
        : public ImageSignerEcDsa
    {
    public:
        ImageSignerEcDsa256(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);

        uint16_t SignatureMethod() const override;
        uint16_t SignatureLength() const override;

    private:
        uECC_Curve GetCurve() const override;

        static const uint16_t signatureMethod = 2;
        static const size_t keyLength = 256;
    };
}

#endif
