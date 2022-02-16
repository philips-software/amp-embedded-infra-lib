#ifndef SERVICES_SHA256_MBEDTLS_HPP
#define SERVICES_SHA256_MBEDTLS_HPP

#include "services/util/Sha256.hpp"
#include "mbedtls/sha256.h"

namespace services
{
    class Sha256MbedTls : public Sha256
    {
    public:
        void Calculate(infra::ConstByteRange input, std::array<uint8_t, 32>& output) override;
    };
}  //! namespace services

#endif  //! SERVICES_SHA256_MBEDTLS_HPP
