#include "upgrade/pack_builder/ImageSignerHashOnly.hpp"
#include "mbedtls/sha256.h"

namespace application
{
    uint16_t ImageSignerHashOnly::SignatureMethod() const
    {
        return signatureMethod;
    }

    uint16_t ImageSignerHashOnly::SignatureLength() const
    {
        return static_cast<uint16_t>(hash.size());
    }

    std::vector<uint8_t> ImageSignerHashOnly::ImageSignature(const std::vector<uint8_t>& image)
    {
        CalculateSha256(image);

        return { hash.begin(), hash.end() };
    }

    bool ImageSignerHashOnly::CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image)
    {
        return signature == ImageSignature(image);
    }

    void ImageSignerHashOnly::CalculateSha256(const std::vector<uint8_t>& image)
    {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, image.data(), image.size());
        mbedtls_sha256_finish(&ctx, hash.data());
        mbedtls_sha256_free(&ctx);
    }
}
