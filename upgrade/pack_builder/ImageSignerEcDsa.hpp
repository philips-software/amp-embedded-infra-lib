#ifndef UPGRADE_IMAGE_SIGNER_EC_DSA_HPP
#define UPGRADE_IMAGE_SIGNER_EC_DSA_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ByteRange.hpp"
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

        std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) override;

    protected:
        virtual void CalculateSignature(const std::vector<uint8_t>& image) = 0;
        virtual std::vector<uint8_t> GetSignature() = 0;
        void CalculateSha256(const std::vector<uint8_t>& image);

        infra::ConstByteRange publicKey;
        infra::ConstByteRange privateKey;
        static hal::SynchronousRandomDataGenerator* randomDataGenerator;
        std::array<uint8_t, 32> hash;

    private:
        static int RandomNumberGenerator(uint8_t* dest, unsigned size);
    };

    class ImageSignerEcDsa224
        : public ImageSignerEcDsa
    {
    public:
        ImageSignerEcDsa224(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);

        uint16_t SignatureMethod() const override;
        uint16_t SignatureLength() const override;
        bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

    protected:
        void CalculateSignature(const std::vector<uint8_t>& image) override;
        std::vector<uint8_t> GetSignature() override;

    private:
        static const uint16_t signatureMethod = 1;
        static const size_t keyLength = 224;
        std::array<uint8_t, keyLength / 8 * 2> signature;
    };

    class ImageSignerEcDsa256
        : public ImageSignerEcDsa
    {
    public:
        ImageSignerEcDsa256(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);

        uint16_t SignatureMethod() const override;
        uint16_t SignatureLength() const override;
        bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

    protected:
        void CalculateSignature(const std::vector<uint8_t>& image) override;
        std::vector<uint8_t> GetSignature() override;

    private:
        static const uint16_t signatureMethod = 2;
        static const size_t keyLength = 256;
        std::array<uint8_t, keyLength / 8 * 2> signature;
    };
}

#endif
