#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SECURITY_XTEA_HMAC_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SECURITY_XTEA_HMAC_HPP

#include "upgrade/pack_builder/ImageAuthenticatorHmac.hpp"
#include "upgrade/pack_builder/ImageEncryptorXtea.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
#include "upgrade/pack_builder/RandomNumberGenerator.hpp"
#include <cstdint>
#include <vector>

namespace application
{
    class ImageSecurityXteaHmac
        : public ImageSecurity
    {
    public:
        static const uint32_t encryptionAndMacMethod = 2;

        ImageSecurityXteaHmac(RandomNumberGenerator& randomNumberGenerator, infra::ConstByteRange xteaKey, infra::ConstByteRange hmacKey);

        virtual uint32_t EncryptionAndMacMethod() const override;
        virtual std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const override;

    private:
        ImageEncryptorXtea encryptor;
        ImageAuthenticatorHmac authenticator;
    };
}

#endif
