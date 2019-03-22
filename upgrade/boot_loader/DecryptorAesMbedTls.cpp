#include "upgrade/boot_loader/DecryptorAesMbedTls.hpp"

namespace application
{
    DecryptorAesMbedTls::DecryptorAesMbedTls(infra::ConstByteRange key)
        : currentStreamBlock()
        , counter()
    {
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_enc(&ctx, key.begin(), key.size() * 8);
    }

    infra::ByteRange DecryptorAesMbedTls::StateBuffer()
    {
        return infra::MakeByteRange(counter);
    }

    void DecryptorAesMbedTls::Reset()
    {
        currentStreamBlockOffset = 0;
    }

    void DecryptorAesMbedTls::DecryptPart(infra::ByteRange data)
    {
        mbedtls_aes_crypt_ctr(&ctx, data.size(), &currentStreamBlockOffset, counter.data(), currentStreamBlock.data(), data.begin(), data.begin());     //TICS !INT#030
    }

    bool DecryptorAesMbedTls::DecryptAndAuthenticate(infra::ByteRange data)
    {
        DecryptPart(data);

        return true;
    }
}
