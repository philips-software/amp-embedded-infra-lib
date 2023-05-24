#ifndef UPGRADE_DECRYPTOR_AES_MBED_TLS_HPP
#define UPGRADE_DECRYPTOR_AES_MBED_TLS_HPP

#include "mbedtls/aes.h"
#include "upgrade/boot_loader/Decryptor.hpp"

namespace application
{
    class DecryptorAesMbedTls
        : public Decryptor
    {
    public:
        static const std::size_t blockLength = 16;

        explicit DecryptorAesMbedTls(infra::ConstByteRange key);

        infra::ByteRange StateBuffer() override;
        void Reset() override;
        void DecryptPart(infra::ByteRange data) override;
        bool DecryptAndAuthenticate(infra::ByteRange data) override;

    private:
        mbedtls_aes_context ctx;
        size_t currentStreamBlockOffset = 0;
        std::array<uint8_t, blockLength> currentStreamBlock;
        std::array<uint8_t, blockLength> counter;
    };
}

#endif
