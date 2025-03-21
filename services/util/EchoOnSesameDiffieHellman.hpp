#ifndef SERVICES_ECHO_ON_SESAME_DIFFIE_HELLMAN_HPP
#define SERVICES_ECHO_ON_SESAME_DIFFIE_HELLMAN_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/EchoOnSesameCrypto.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/EchoOnSesameCryptoMbedTls.hpp"
#endif
#include "services/util/SesameSecured.hpp"

namespace services
{
    class EchoOnSesameDiffieHellman
        : public EchoOnSesame
        , private sesame_security::DiffieHellmanKeyEstablishment
        , private sesame_security::DiffieHellmanKeyEstablishmentProxy
    {
    public:
        struct Crypto
        {
            infra::CreatorBase<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchange;
            EcSecP256r1DsaSigner& signer;
            infra::CreatorBase<EcSecP256r1DsaVerifier, void(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate)>& verifier;
            HmacDrbgSha256& keyExpander;
        };

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif

        EchoOnSesameDiffieHellman(const Crypto& crypto, SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of SesameObserver
        void Initialized() override;

    private:
        // Implementation of DiffieHellmanKeyEstablishment
        void Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS) override;
        void PresentCertificate(infra::BoundedConstString otherDsaCertificate) override;

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

        infra::CreatorBase<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchangeCreator;
        infra::Optional<infra::ProxyCreator<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>> keyExchange;
        EcSecP256r1DsaSigner& signer;
        infra::CreatorBase<EcSecP256r1DsaVerifier, void(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate)>& verifierCreator;
        infra::Optional<infra::ProxyCreator<EcSecP256r1DsaVerifier, void(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate)>> verifier;
        HmacDrbgSha256& keyExpander;
    };

#ifdef EMIL_USE_MBEDTLS
    struct EchoOnSesameDiffieHellman::WithCryptoMbedTls
        : public EchoOnSesameDiffieHellman
    {
        WithCryptoMbedTls(SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString dsaCertificatePrivateKey, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        infra::Creator<EcSecP256r1DiffieHellman, EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<EcSecP256r1DsaVerifier, EcSecP256r1DsaVerifierMbedTls, void(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate)> verifier;
        HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
