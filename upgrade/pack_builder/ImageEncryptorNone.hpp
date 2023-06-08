#ifndef UPGRADE_IMAGE_ENCRYPTOR_NONE_HPP
#define UPGRADE_IMAGE_ENCRYPTOR_NONE_HPP

#include "upgrade/pack_builder/ImageSecurity.hpp"
#include <cstdint>
#include <vector>

namespace application
{
    class ImageEncryptorNone
        : public ImageSecurity
    {
    public:
        static const uint32_t encryptionAndMacMethod = 0;

        uint32_t EncryptionAndMacMethod() const override;
        std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const override;
    };
}

#endif
