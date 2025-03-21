#ifndef SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP
#define SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameSecured.hpp"

namespace services
{
    class EcSecP256r1DiffieHellman
    {
    public:
        EcSecP256r1DiffieHellman(hal::SynchronousRandomDataGenerator& randomDataGenerator);
        ~EcSecP256r1DiffieHellman();

        std::array<uint8_t, 65> PublicKey() const;
        std::array<uint8_t, 32> SharedSecret(infra::ConstByteRange otherPublicKey, hal::SynchronousRandomDataGenerator& randomDataGenerator) const;

    private:
        mbedtls_ecp_group group;
        mbedtls_mpi privateKey;
        mbedtls_ecp_point publicKey;

    private:
        static int StaticRng(void* self, unsigned char* data, std::size_t size);

        mutable hal::SynchronousRandomDataGenerator* rng = nullptr;
    };

    class EcSecP256r1DsaSigner
    {
    public:
        EcSecP256r1DsaSigner(infra::BoundedConstString dsaCertificatePrivateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator);
        ~EcSecP256r1DsaSigner();

        std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> Sign(infra::ConstByteRange data, hal::SynchronousRandomDataGenerator& randomDataGenerator) const;
        bool Verify(infra::ConstByteRange data, const mbedtls_ecp_point& otherDsaPublicKey, infra::ConstByteRange r, infra::ConstByteRange s) const;

    private:
        mbedtls_ecp_group group;
        mbedtls_mpi privateKey;
        mbedtls_ecdh_context context;

    private:
        static int StaticRng(void* self, unsigned char* data, std::size_t size);

        mutable hal::SynchronousRandomDataGenerator* rng = nullptr;
    };

    class EcSecP256r1DsaVerifier
    {
    public:
        EcSecP256r1DsaVerifier(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate);
        ~EcSecP256r1DsaVerifier();

        bool Verify(infra::ConstByteRange data, infra::ConstByteRange r, infra::ConstByteRange s) const;

    private:
        mbedtls_x509_crt rootCertificate;
        mbedtls_ecp_group group;
        mbedtls_ecp_point publicKey;
    };

    class HmacDrbgSha256
    {
    public:
        void Expand(infra::ConstByteRange seed, infra::ByteRange expandedMaterial) const;
    };

    class EchoOnSesameDiffieHellman
        : public EchoOnSesame
        , private sesame_security::DiffieHellmanKeyEstablishment
        , private sesame_security::DiffieHellmanKeyEstablishmentProxy
    {
    public:
        EchoOnSesameDiffieHellman(SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString dsaCertificatePrivateKey, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of SesameObserver
        void Initialized() override;

    private:
        // Implementation of DiffieHellmanKeyEstablishment
        void Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS) override;
        void PresentCertificate(infra::BoundedConstString dsaCertificate) override;

        void ReQueueWaitingProxies();

    private:
        SesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::BoundedConstString dsaCertificate;
        infra::BoundedConstString rootCaCertificate;

        bool initializingKeys = true;
        bool sentExchange = false;
        infra::Optional<std::pair<std::array<uint8_t, 16>, std::array<uint8_t, 16>>> nextKeyPair;
        infra::IntrusiveList<ServiceProxy> waitingProxies;

        infra::Optional<EcSecP256r1DiffieHellman> keyExchange;
        EcSecP256r1DsaSigner signer;
        infra::Optional<EcSecP256r1DsaVerifier> verifier;
        HmacDrbgSha256 keyExpander;
    };
}

#endif
