#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SECURITY_XTEA_HMAC_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SECURITY_XTEA_HMAC_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "upgrade/pack_builder/ImageAuthenticatorHmac.hpp"
#include "upgrade/pack_builder/ImageEncryptorXtea.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
#include <cstdint>
#include <vector>

namespace application
{
    class ImageSecurityXteaHmac
        : public ImageSecurity
    {
    public:
        static const uint32_t encryptionAndMacMethod = 2;

        ImageSecurityXteaHmac(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange xteaKey, infra::ConstByteRange hmacKey);

        virtual uint32_t EncryptionAndMacMethod() const override;
        virtual std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const override;

    private:
        ImageEncryptorXtea encryptor;
        ImageAuthenticatorHmac authenticator;
    };
}

#endif
