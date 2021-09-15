#include "upgrade/pack_builder/ImageAuthenticatorHmac.hpp"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
#include <algorithm>
#include <fstream>
#include <iterator>

namespace application
{
    ImageAuthenticatorHmac::ImageAuthenticatorHmac(infra::ConstByteRange key)
        : key(key)
    {}

    std::vector<uint8_t> ImageAuthenticatorHmac::GenerateMac(const std::vector<uint8_t>& data) const
    {
        std::vector<uint8_t> mac(32, 0);

        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
        mbedtls_md_hmac_starts(&ctx, key.begin(), key.size());
        mbedtls_md_hmac_update(&ctx, data.data(), data.size());
        mbedtls_md_hmac_finish(&ctx, mac.data());
        mbedtls_md_free(&ctx);

        return mac;
    }
}
