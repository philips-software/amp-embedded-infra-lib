#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGE_ENCRYPTOR_XTEA_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGE_ENCRYPTOR_XTEA_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "upgrade/pack_builder/ImageAuthenticatorHmac.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
#include <cstdint>
#include <vector>

namespace application
{
    class ImageEncryptorXtea
    {
    public:
        static const std::size_t blockLength = 8;

        ImageEncryptorXtea(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange key);

        std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const;

    private:
        bool CheckDecryption(const std::vector<uint8_t>& original, const std::vector<uint8_t>& encrypted) const;

        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::ConstByteRange key;
    };
}

#endif
