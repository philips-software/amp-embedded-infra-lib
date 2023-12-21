#ifndef UPGRADE_DECRYPTOR_NONE_HPP
#define UPGRADE_DECRYPTOR_NONE_HPP

#include "upgrade/boot_loader/Decryptor.hpp"

namespace application
{
    class DecryptorNone
        : public Decryptor
    {
    public:
        infra::ByteRange StateBuffer() override;
        void Reset() override;
        void DecryptPart(infra::ByteRange data) override;
        bool DecryptAndAuthenticate(infra::ByteRange data) override;
    };
}

#endif
