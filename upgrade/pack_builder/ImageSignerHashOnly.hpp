#ifndef UPGRADE_IMAGE_SIGNER_HASH_ONLY_HPP
#define UPGRADE_IMAGE_SIGNER_HASH_ONLY_HPP

#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/ImageSigner.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace application
{
    class ImageSignerHashOnly
        : public ImageSigner
    {
    public:
        uint16_t SignatureMethod() const override;
        uint16_t SignatureLength() const override;
        std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) override;
        bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

    private:
        void CalculateSha256(const std::vector<uint8_t>& image);

    private:
        static const uint16_t signatureMethod = 0;
        static const size_t hashLength = 32;

        std::array<uint8_t, hashLength> hash;
    };
}

#endif
