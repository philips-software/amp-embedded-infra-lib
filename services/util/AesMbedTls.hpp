#ifndef SERVICES_AES_MBEDTLS_HPP
#define SERVICES_AES_MBEDTLS_HPP

#include "services/util/Aes.hpp"
#include "mbedtls/aes.h"

namespace services
{
    class AesMbedTls : public Aes
    {
    public:
        AesMbedTls();

        void AddKey(const std::array<uint8_t, 16>& key) override;
        void Encrypt(std::array<uint8_t, 16>& iv, infra::ConstByteRange input, infra::ByteRange output) override;
        void Decrypt(std::array<uint8_t, 16>& iv, infra::ConstByteRange input, infra::ByteRange output) override;

    private:
        mbedtls_aes_context context;
    };
}  //! namespace services

#endif  //! SERVICES_AES_MBEDTLS_HPP
