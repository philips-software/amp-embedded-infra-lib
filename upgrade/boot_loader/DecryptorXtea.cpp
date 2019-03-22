#include "upgrade/boot_loader/DecryptorXtea.hpp"

namespace application
{
    DecryptorXtea::DecryptorXtea(infra::ConstByteRange key)
    {
        mbedtls_xtea_init(&ctx);
        assert(key.size() == 16);
        mbedtls_xtea_setup(&ctx, key.begin());                                                                  //TICS !INT#030
    }

    infra::ByteRange DecryptorXtea::StateBuffer()
    {
        return infra::MakeByteRange(iv);
    }

    void DecryptorXtea::Reset()
    {}

    void DecryptorXtea::DecryptPart(infra::ByteRange data)
    {
        mbedtls_xtea_crypt_cbc(&ctx, MBEDTLS_XTEA_DECRYPT, data.size(), iv.data(), data.begin(), data.begin()); //TICS !INT#030
    }

    bool DecryptorXtea::DecryptAndAuthenticate(infra::ByteRange data)
    {
        DecryptPart(data);

        return true;
    }
}
