#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGE_AUTHENTICATOR_HMAC_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGE_AUTHENTICATOR_HMAC_HPP

#include "infra/util/ByteRange.hpp"
#include <cstdint>
#include <vector>

namespace application
{
    class ImageAuthenticatorHmac
    {
    public:
        explicit ImageAuthenticatorHmac(infra::ConstByteRange key);

        std::vector<uint8_t> GenerateMac(const std::vector<uint8_t>& data) const;

    private:
        infra::ConstByteRange key;
    };
}

#endif
