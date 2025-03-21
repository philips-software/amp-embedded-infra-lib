#ifndef SERVICES_ECHO_ON_SESAME_CRYPTO_HPP
#define SERVICES_ECHO_ON_SESAME_CRYPTO_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/BoundedString.hpp"

namespace services
{
    class EcSecP256r1DiffieHellman
    {
    public:
        EcSecP256r1DiffieHellman() = default;
        EcSecP256r1DiffieHellman(const EcSecP256r1DiffieHellman& other) = delete;
        EcSecP256r1DiffieHellman& operator=(const EcSecP256r1DiffieHellman& other) = delete;

        virtual std::array<uint8_t, 65> PublicKey() const = 0;
        virtual std::array<uint8_t, 32> SharedSecret(infra::ConstByteRange otherPublicKey, hal::SynchronousRandomDataGenerator& randomDataGenerator) const = 0;
    };

    class EcSecP256r1DsaSigner
    {
    public:
        EcSecP256r1DsaSigner() = default;
        EcSecP256r1DsaSigner(const EcSecP256r1DsaSigner& other) = delete;
        EcSecP256r1DsaSigner& operator=(const EcSecP256r1DsaSigner& other) = delete;

        virtual std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> Sign(infra::ConstByteRange data, hal::SynchronousRandomDataGenerator& randomDataGenerator) const = 0;
    };

    class EcSecP256r1DsaVerifier
    {
    public:
        EcSecP256r1DsaVerifier() = default;
        EcSecP256r1DsaVerifier(const EcSecP256r1DsaVerifier& other) = delete;
        EcSecP256r1DsaVerifier& operator=(const EcSecP256r1DsaVerifier& other) = delete;

        virtual bool Verify(infra::ConstByteRange data, infra::ConstByteRange r, infra::ConstByteRange s) const = 0;
    };

    class HmacDrbgSha256
    {
    public:
        HmacDrbgSha256() = default;
        HmacDrbgSha256(const HmacDrbgSha256& other) = delete;
        HmacDrbgSha256& operator=(const HmacDrbgSha256& other) = delete;

        virtual void Expand(infra::ConstByteRange seed, infra::ByteRange expandedMaterial) const = 0;
    };
}

#endif
