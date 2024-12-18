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

    class Aes128Ecb
    {
    public:
        Aes128Ecb() = default;
        Aes128Ecb(const Aes128Ecb& other) = delete;
        Aes128Ecb& operator=(const Aes128Ecb& other) = delete;

        virtual void SetKey(const std::array<uint8_t, 16>& key) const = 0;
        virtual void Encrypt(infra::ConstByteRange input, infra::ByteRange output) const = 0;
        virtual void Decrypt(infra::ConstByteRange input, infra::ByteRange output) const = 0;
    };

    class Aes128Cmac
    {
    public:
        Aes128Cmac() = default;
        Aes128Cmac(const Aes128Cmac& other) = delete;
        Aes128Cmac& operator=(const Aes128Cmac& other) = delete;

        using Mac = std::array<uint8_t, 16>;

        virtual void SetKey(const std::array<uint8_t, 16>& key) = 0;
        virtual void Append(infra::ConstByteRange input) = 0;
        virtual Mac Calculate() = 0;
    };
}

#endif
