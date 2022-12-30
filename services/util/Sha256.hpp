#ifndef SERVICES_SHA256_HPP
#define SERVICES_SHA256_HPP

#include "infra/util/ByteRange.hpp"

namespace services
{
    class Sha256
    {
    public:
        Sha256() = default;
        Sha256(const Sha256& other) = delete;
        Sha256& operator=(const Sha256& other) = delete;
        virtual ~Sha256() = default;

        using Digest = std::array<uint8_t, 32>;

        virtual Digest Calculate(infra::ConstByteRange input) const = 0;
    };
}

#endif  //! SERVICES_SHA256_HPP
