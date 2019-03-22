#ifndef UPGRADE_DECRYPTOR_AES_TINY_HPP
#define UPGRADE_DECRYPTOR_AES_TINY_HPP

#include "upgrade/boot_loader/Decryptor.hpp"

namespace application
{
    class DecryptorAesTiny
        : public Decryptor
    {
    public:
        static const std::size_t blockLength = 16;

        explicit DecryptorAesTiny(infra::ConstByteRange key);

        virtual infra::ByteRange StateBuffer() override;
        virtual void Reset() override;
        virtual void DecryptPart(infra::ByteRange data) override;
        virtual bool DecryptAndAuthenticate(infra::ByteRange data) override;

    private:
        void IncreaseCounter();

    private:
        size_t currentStreamBlockOffset = 0;
        std::array<uint8_t, blockLength> currentStreamBlock;
        std::array<uint8_t, blockLength> counter;
    };
}

#endif
