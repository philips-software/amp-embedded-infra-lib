#ifndef SERVICES_AES_HPP
#define SERVICES_AES_HPP

#include "infra/util/ByteRange.hpp"

namespace services
{
    class Aes
    {
    public:
        Aes() {}
        virtual ~Aes() {}

        virtual void AddKey(const std::array<uint8_t, 16>& key) = 0;
        virtual void Encrypt(std::array<uint8_t, 16>& iv, infra::ConstByteRange input, infra::ByteRange output) = 0;
        virtual void Decrypt(std::array<uint8_t, 16>& iv, infra::ConstByteRange input, infra::ByteRange output) = 0;

    protected:
        Aes(const Aes& other) = delete;
        Aes& operator=(const Aes& other) = delete;
    };
}  //! namespace services

#endif  //! SERVICES_AES_HPP