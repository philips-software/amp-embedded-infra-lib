#ifndef SERVICES_SHA256_MBEDTLS_HPP
#define SERVICES_SHA256_MBEDTLS_HPP

#include "services/util/Sha256.hpp"

namespace services
{
    class Sha256MbedTls
        : public Sha256
    {
    public:
        std::array<uint8_t, 32> Calculate(infra::ConstByteRange input) const override;
    };
}

#endif
