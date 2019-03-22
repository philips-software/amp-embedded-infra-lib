#ifndef UPGRADE_DECRYPTOR_XTEA_HPP
#define UPGRADE_DECRYPTOR_XTEA_HPP

#include "mbedtls/xtea.h"
#include "upgrade/boot_loader/Decryptor.hpp"

namespace application
{
    class DecryptorXtea
        : public Decryptor
    {
    public:
        static const std::size_t blockLength = 8;

        explicit DecryptorXtea(infra::ConstByteRange key);

        virtual infra::ByteRange StateBuffer() override;
        virtual void Reset() override;
        virtual void DecryptPart(infra::ByteRange data) override;
        virtual bool DecryptAndAuthenticate(infra::ByteRange data) override;

    private:
        mbedtls_xtea_context ctx;
        std::array<uint8_t, blockLength> iv;
    };
}

#endif
