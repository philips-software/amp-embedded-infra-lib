#include "services/util/AesMbedTls.hpp"

namespace services
{
    Aes128CtrMbedTls::Aes128CtrMbedTls()
    {
        mbedtls_aes_init(&context);
    }

    void Aes128CtrMbedTls::SetKey(const std::array<uint8_t, 16>& key)
    {
        mbedtls_aes_setkey_enc(&context, &key[0], key.size() * 8);
    }

    void Aes128CtrMbedTls::Encrypt(std::array<uint8_t, 16>& counter, infra::ConstByteRange input, infra::ByteRange output)
    {
        size_t currentStreamBlockOffset = 0;
        std::array<uint8_t, 16> currentStreamBlock{};
        mbedtls_aes_crypt_ctr(&context, input.size(), &currentStreamBlockOffset, counter.data(), currentStreamBlock.data(), input.begin(), output.begin());
    }
}
