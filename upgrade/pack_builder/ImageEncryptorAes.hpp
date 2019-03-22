#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGEUPGRADE_PACK_BUILD_LIBRARY_ENCRYPTOR_AES_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGEUPGRADE_PACK_BUILD_LIBRARY_ENCRYPTOR_AES_HPP

#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
#include "upgrade/pack_builder/RandomNumberGenerator.hpp"
#include <cstdint>
#include <vector>

namespace application
{
    class ImageEncryptorAes
        : public ImageSecurity
    {
    public:
        static const uint32_t encryptionAndMacMethod = 1;
        static const std::size_t blockLength = 16;

        ImageEncryptorAes(RandomNumberGenerator& randomNumberGenerator, infra::ConstByteRange key);

        virtual uint32_t EncryptionAndMacMethod() const override;
        virtual std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const override;

    private:
        bool CheckDecryption(const std::vector<uint8_t>& original, const std::vector<uint8_t>& encrypted) const;

        RandomNumberGenerator& randomNumberGenerator;
        infra::ConstByteRange key;
    };
}

#endif
