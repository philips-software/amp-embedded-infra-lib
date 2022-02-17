#ifndef SERVICES_AES_HPP
#define SERVICES_AES_HPP

#include "infra/util/ByteRange.hpp"

namespace services
{
    class Aes128Ctr
    {
    public:
        Aes128Ctr() = default;
        Aes128Ctr(const Aes128Ctr& other) = delete;
        Aes128Ctr& operator=(const Aes128Ctr& other) = delete;
        virtual ~Aes128Ctr() = default;

        virtual void SetKey(const std::array<uint8_t, 16>& key) = 0;
        virtual void Encrypt(std::array<uint8_t, 16>& counter, infra::ConstByteRange input, infra::ByteRange output) = 0;
    };
}

#endif  //! SERVICES_AES_HPP