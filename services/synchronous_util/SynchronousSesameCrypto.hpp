#ifndef SERVICES_SYNCHRONOUS_SESAME_CRYPTO_HPP
#define SERVICES_SYNCHRONOUS_SESAME_CRYPTO_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/BoundedString.hpp"

namespace services
{
    class SynchronousEcSecP256r1DiffieHellman
    {
    public:
        SynchronousEcSecP256r1DiffieHellman() = default;
        SynchronousEcSecP256r1DiffieHellman(const SynchronousEcSecP256r1DiffieHellman& other) = delete;
        SynchronousEcSecP256r1DiffieHellman& operator=(const SynchronousEcSecP256r1DiffieHellman& other) = delete;

        virtual std::array<uint8_t, 65> PublicKey() const = 0;
        virtual std::array<uint8_t, 32> SharedSecret(infra::ConstByteRange otherPublicKey) const = 0;
    };

    class SynchronousEcSecP256r1DsaSigner
    {
    public:
        SynchronousEcSecP256r1DsaSigner() = default;
        SynchronousEcSecP256r1DsaSigner(const SynchronousEcSecP256r1DsaSigner& other) = delete;
        SynchronousEcSecP256r1DsaSigner& operator=(const SynchronousEcSecP256r1DsaSigner& other) = delete;

        virtual std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> Sign(infra::ConstByteRange data) const = 0;
    };

    class SynchronousEcSecP256r1DsaVerifier
    {
    public:
        SynchronousEcSecP256r1DsaVerifier() = default;
        SynchronousEcSecP256r1DsaVerifier(const SynchronousEcSecP256r1DsaVerifier& other) = delete;
        SynchronousEcSecP256r1DsaVerifier& operator=(const SynchronousEcSecP256r1DsaVerifier& other) = delete;

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

    class SynchronousAesGcmEncryption
    {
    public:
        SynchronousAesGcmEncryption() = default;
        SynchronousAesGcmEncryption(const SynchronousAesGcmEncryption& other) = delete;
        SynchronousAesGcmEncryption& operator=(const SynchronousAesGcmEncryption& other) = delete;

        virtual void EncryptWithKey(infra::ConstByteRange key) = 0;
        virtual void DecryptWithKey(infra::ConstByteRange key) = 0;
        virtual void Start(infra::ConstByteRange iv) = 0;
        virtual std::size_t Update(infra::ConstByteRange from, infra::ByteRange to) = 0;
        virtual std::size_t Finish(infra::ByteRange to, infra::ByteRange mac) = 0;
    };
}

#endif
