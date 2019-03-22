#include "mbedtls/xtea.h"
#include "upgrade/pack_builder/ImageSecurityXteaHmac.hpp"
#include <cassert>
#include <algorithm>
#include <array>

namespace application
{
    ImageSecurityXteaHmac::ImageSecurityXteaHmac(RandomNumberGenerator& randomNumberGenerator, infra::ConstByteRange xteaKey, infra::ConstByteRange hmacKey)
        : encryptor(randomNumberGenerator, xteaKey)
        , authenticator(hmacKey)
    {}

    uint32_t ImageSecurityXteaHmac::EncryptionAndMacMethod() const
    {
        return encryptionAndMacMethod;
    }

    std::vector<uint8_t> ImageSecurityXteaHmac::Secure(const std::vector<uint8_t>& data) const
    {
        std::vector<uint8_t> result = encryptor.Secure(data);

        std::vector<uint8_t> mac = authenticator.GenerateMac(result);
        result.insert(result.begin(), mac.begin(), mac.end());

        return result;
    }
}
