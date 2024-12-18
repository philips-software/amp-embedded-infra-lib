#ifndef SERVICES_AES_CMAC_HPP
#define SERVICES_AES_CMAC_HPP

#include "services/util/Aes.hpp"

namespace services
{
    class Aes128CmacImpl
        : public Aes128Cmac
    {
    public:
        explicit Aes128CmacImpl(Aes128Ecb& aes128Ecb);

        void SetKey(const std::array<uint8_t, 16>& key) override;
        void Append(infra::ConstByteRange input) override;
        Mac Calculate() override;

    private:
        void ComputeK1(infra::ByteRange key);
        void ComputeK2(infra::ByteRange block, infra::ByteRange key);

    private:
        const static std::size_t blockSize = 16;
        Aes128Ecb& aes128Ecb;
        std::array<uint8_t, blockSize> tag;
        std::array<uint8_t, blockSize> lastBlock;
        std::size_t lastBlockSize = 0;
    };
}

#endif
