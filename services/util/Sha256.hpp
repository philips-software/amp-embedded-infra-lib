#ifndef SERVICES_SHA256_HPP
#define SERVICES_SHA256_HPP

#include "infra/util/ByteRange.hpp"

namespace services
{
    class Sha256
    {
    public:
        virtual void Calculate(infra::ConstByteRange input, std::array<uint8_t, 32>& output) = 0;

    protected:
        Sha256() = default;
        Sha256(const Sha256& other) = delete;
        Sha256& operator=(const Sha256& other) = delete;
        virtual ~Sha256() = default;
    };
}  //! namespace services

#endif  //! SERVICES_SHA256_HPP
