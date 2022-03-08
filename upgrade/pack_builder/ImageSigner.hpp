#ifndef UPGRADE_IMAGE_SIGNER_HPP
#define UPGRADE_IMAGE_SIGNER_HPP

#include <cstdint>
#include <vector>

namespace application
{
    class ImageSigner
    {
    public:
        virtual uint16_t SignatureMethod() const = 0;
        virtual uint16_t SignatureLength() const = 0;
        virtual std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) = 0;
        virtual bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) = 0;

    protected:
        ~ImageSigner() = default;
    };
}

#endif
