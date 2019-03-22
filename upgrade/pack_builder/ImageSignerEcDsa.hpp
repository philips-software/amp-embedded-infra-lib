#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SIGNER_EC_DSA_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SIGNER_EC_DSA_HPP

#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/ImageSigner.hpp"
#include "upgrade/pack_builder/RandomNumberGenerator.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace application
{
    class ImageSignerEcDsa
        : public ImageSigner
    {
    public:
        ImageSignerEcDsa(RandomNumberGenerator& randomNumberGenerator, infra::ConstByteRange publicKey, infra::ConstByteRange privateKey);

        virtual uint16_t SignatureMethod() const override;
        virtual uint16_t SignatureLength() const override;
        virtual std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) override;
        virtual bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

        static const uint16_t signatureMethod = 1;
        static const size_t keyLength = 224;

    private:
        void CalculateSha256(const std::vector<uint8_t>& image);
        void CalculateSignature(const std::vector<uint8_t>& image);

        static int MyRandom(uint8_t* dest, unsigned size);

    private:
        infra::ConstByteRange publicKey;
        infra::ConstByteRange privateKey;
        static RandomNumberGenerator* randomNumberGenerator;
        std::string keyFileName;
        std::array<uint8_t, 32> hash;
        std::array<uint8_t, keyLength / 8 * 2> signature;
    };
}

#endif
