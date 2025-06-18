#ifndef SERVICES_SESAME_CRYPTO_MBED_TLS_HPP
#define SERVICES_SESAME_CRYPTO_MBED_TLS_HPP

#include "infra/util/BoundedVector.hpp"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/gcm.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "services/util/SesameCrypto.hpp"

namespace services
{
    class EcSecP256r1DiffieHellmanMbedTls
        : public EcSecP256r1DiffieHellman
    {
    public:
        explicit EcSecP256r1DiffieHellmanMbedTls(hal::SynchronousRandomDataGenerator& randomDataGenerator);
        ~EcSecP256r1DiffieHellmanMbedTls();

        std::array<uint8_t, 65> PublicKey() const override;
        std::array<uint8_t, 32> SharedSecret(infra::ConstByteRange otherPublicKey) const override;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        mbedtls_ecp_group group;
        mbedtls_mpi privateKey;
        mbedtls_ecp_point publicKey;
    };

    class EcSecP256r1DsaSignerMbedTls
        : public EcSecP256r1DsaSigner
    {
    public:
        EcSecP256r1DsaSignerMbedTls(infra::ConstByteRange dsaCertificatePrivateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator);
        ~EcSecP256r1DsaSignerMbedTls();

        std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> Sign(infra::ConstByteRange data) const override;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        mbedtls_ecp_group group;
        mbedtls_mpi privateKey;
        mbedtls_ecdh_context context;
    };

    class EcSecP256r1DsaVerifierMbedTls
        : public EcSecP256r1DsaVerifier
    {
    public:
        EcSecP256r1DsaVerifierMbedTls(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate);
        ~EcSecP256r1DsaVerifierMbedTls();

        bool Verify(infra::ConstByteRange data, infra::ConstByteRange r, infra::ConstByteRange s) const override;

    private:
        mbedtls_x509_crt rootCertificate;
        mbedtls_ecp_group group;
        mbedtls_ecp_point publicKey;

        bool valid = false;
    };

    class HmacDrbgSha256MbedTls
        : public HmacDrbgSha256
    {
    public:
        void Expand(infra::ConstByteRange seed, infra::ByteRange expandedMaterial) const override;
    };

    class AesGcmEncryptionMbedTls
        : public AesGcmEncryption
    {
    public:
        AesGcmEncryptionMbedTls();
        ~AesGcmEncryptionMbedTls();

        void EncryptWithKey(infra::ConstByteRange key) override;
        void DecryptWithKey(infra::ConstByteRange key) override;
        void Start(infra::ConstByteRange iv) override;
        std::size_t Update(infra::ConstByteRange from, infra::ByteRange to) override;
        std::size_t Finish(infra::ByteRange to, infra::ByteRange mac) override;

    private:
        bool encrypt = false;
        mbedtls_gcm_context context;
    };

    class EcSecP256r1PrivateKey
    {
    public:
        explicit EcSecP256r1PrivateKey(hal::SynchronousRandomDataGenerator& randomDataGenerator);
        explicit EcSecP256r1PrivateKey(infra::ConstByteRange key, hal::SynchronousRandomDataGenerator& randomDataGenerator);
        EcSecP256r1PrivateKey(const EcSecP256r1PrivateKey& other) = delete;
        EcSecP256r1PrivateKey& operator=(const EcSecP256r1PrivateKey& other) = delete;
        ~EcSecP256r1PrivateKey();

        infra::BoundedString::WithStorage<228> Pem() const;
        std::array<uint8_t, 121> Der() const;
        const mbedtls_pk_context& Context() const;

    private:
        mbedtls_pk_context context;
    };

    class EcSecP256r1Certificate
    {
    public:
        EcSecP256r1Certificate(const EcSecP256r1PrivateKey& subjectKey, const char* subjectName, const EcSecP256r1PrivateKey& issuerKey, const char* issuerName, hal::SynchronousRandomDataGenerator& randomDataGenerator);
        EcSecP256r1Certificate(const EcSecP256r1Certificate& other) = delete;
        EcSecP256r1Certificate& operator=(const EcSecP256r1Certificate& other) = delete;
        ~EcSecP256r1Certificate();

        infra::BoundedString::WithStorage<512> Pem() const;
        infra::BoundedVector<uint8_t>::WithMaxSize<512> Der() const;

    private:
        mbedtls_x509write_cert dsaCertificate;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
    };
}

#endif
