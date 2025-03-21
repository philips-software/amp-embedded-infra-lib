#ifndef SERVICES_ECHO_ON_SESAME_CRYPTO_MBED_TLS_HPP
#define SERVICES_ECHO_ON_SESAME_CRYPTO_MBED_TLS_HPP

#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "services/util/EchoOnSesameCrypto.hpp"

namespace services
{
    class EcSecP256r1DiffieHellmanMbedTls
        : public EcSecP256r1DiffieHellman
    {
    public:
        EcSecP256r1DiffieHellmanMbedTls(hal::SynchronousRandomDataGenerator& randomDataGenerator);
        ~EcSecP256r1DiffieHellmanMbedTls();

        std::array<uint8_t, 65> PublicKey() const override;
        std::array<uint8_t, 32> SharedSecret(infra::ConstByteRange otherPublicKey) const override;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        mbedtls_ecp_group group;
        mbedtls_mpi privateKey;
        mbedtls_ecp_point publicKey;

    private:
        static int StaticRng(void* self, unsigned char* data, std::size_t size);

        mutable hal::SynchronousRandomDataGenerator* rng = nullptr;
    };

    class EcSecP256r1DsaSignerMbedTls
        : public EcSecP256r1DsaSigner
    {
    public:
        EcSecP256r1DsaSignerMbedTls(infra::BoundedConstString dsaCertificatePrivateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator);
        ~EcSecP256r1DsaSignerMbedTls();

        std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> Sign(infra::ConstByteRange data) const override;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        mbedtls_ecp_group group;
        mbedtls_mpi privateKey;
        mbedtls_ecdh_context context;

    private:
        static int StaticRng(void* self, unsigned char* data, std::size_t size);

        mutable hal::SynchronousRandomDataGenerator* rng = nullptr;
    };

    class EcSecP256r1DsaVerifierMbedTls
        : public EcSecP256r1DsaVerifier
    {
    public:
        EcSecP256r1DsaVerifierMbedTls(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate);
        ~EcSecP256r1DsaVerifierMbedTls();

        bool Verify(infra::ConstByteRange data, infra::ConstByteRange r, infra::ConstByteRange s) const override;

    private:
        mbedtls_x509_crt rootCertificate;
        mbedtls_ecp_group group;
        mbedtls_ecp_point publicKey;
    };

    class HmacDrbgSha256MbedTls
        : public HmacDrbgSha256
    {
    public:
        void Expand(infra::ConstByteRange seed, infra::ByteRange expandedMaterial) const override;
    };
}

#endif
