#include "services/util/AesMbedTls.hpp"

namespace services
{
    AesMbedTls::AesMbedTls()
    {
        mbedtls_aes_init(&context);
    }

    void AesMbedTls::AddKey(const std::array<uint8_t, 16>& key)
    {
        mbedtls_aes_setkey_enc(&context, &key[0], key.size() * 8);
    }

    void AesMbedTls::Encrypt(std::array<uint8_t, 16>& iv, infra::ConstByteRange input, infra::ByteRange output)
    {
        size_t currentStreamBlockOffset = 0;
        std::array<uint8_t, 16> currentStreamBlock{};
        mbedtls_aes_crypt_ctr(&context, input.size(), &currentStreamBlockOffset, iv.data(), currentStreamBlock.data(), input.begin(), output.begin());
    }

    void AesMbedTls::Decrypt(std::array<uint8_t, 16>& iv, infra::ConstByteRange input, infra::ByteRange output)
    {
        Encrypt(iv, input, output);
    }
}
