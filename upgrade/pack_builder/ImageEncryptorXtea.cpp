#include "upgrade/pack_builder/ImageEncryptorXtea.hpp"
#include "mbedtls/xtea.h"
#include <algorithm>
#include <array>
#include <cassert>

namespace application
{
    ImageEncryptorXtea::ImageEncryptorXtea(hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::ConstByteRange key)
        : randomDataGenerator(randomDataGenerator)
        , key(key)
    {}

    std::vector<uint8_t> ImageEncryptorXtea::Secure(const std::vector<uint8_t>& unalignedData) const
    {
        std::vector<uint8_t> data = unalignedData;
        data.resize(data.size() + blockLength - (data.size() - 1 + blockLength) % blockLength - 1, 0);
        std::vector<uint8_t> iv(blockLength, 0);
        randomDataGenerator.GenerateRandomData(iv);

        mbedtls_xtea_context ctx;
        mbedtls_xtea_init(&ctx);
        assert(key.size() == 16);
        mbedtls_xtea_setup(&ctx, key.begin());

        std::vector<uint8_t> result = iv;
        result.resize(result.size() + data.size());

        int ret = mbedtls_xtea_crypt_cbc(&ctx, MBEDTLS_XTEA_ENCRYPT, data.size(), iv.data(), data.data(), result.data() + blockLength);
        if (ret != 0)
            throw std::runtime_error("XTEA encryption failed");

        if (!CheckDecryption(data, result))
            throw std::runtime_error("XTEA decryption check failed");

        return result;
    }

    bool ImageEncryptorXtea::CheckDecryption(const std::vector<uint8_t>& original, const std::vector<uint8_t>& encrypted) const
    {
        std::vector<uint8_t> iv(encrypted.begin(), encrypted.begin() + blockLength);

        mbedtls_xtea_context ctx;
        mbedtls_xtea_init(&ctx);
        mbedtls_xtea_setup(&ctx, key.begin());

        if (encrypted.size() % blockLength != 0)
            return false;

        std::vector<uint8_t> decrypted(encrypted.size() - blockLength, 0);

        int ret = mbedtls_xtea_crypt_cbc(&ctx, MBEDTLS_XTEA_DECRYPT, decrypted.size(), iv.data(), encrypted.data() + blockLength, decrypted.data());

        return ret == 0 && mbedtls_xtea_self_test(0) == 0 && decrypted == original;
    }
}
