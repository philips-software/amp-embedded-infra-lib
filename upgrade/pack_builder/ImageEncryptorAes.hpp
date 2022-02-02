#ifndef UPGRADE_IMAGE_ENCRYPTOR_AES_HPP
#define UPGRADE_IMAGE_ENCRYPTOR_AES_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
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

        ImageEncryptorAes(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange key);

        virtual uint32_t EncryptionAndMacMethod() const override;
        virtual std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const override;

    private:
        bool CheckDecryption(const std::vector<uint8_t>& original, const std::vector<uint8_t>& encrypted) const;

        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::ConstByteRange key;
    };
}

#endif
