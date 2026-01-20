#ifndef SERVICES_ECHO_POLICY_DIFFIE_HELLMAN_HPP
#define SERVICES_ECHO_POLICY_DIFFIE_HELLMAN_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameCrypto.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif
#include "services/util/SesameSecured.hpp"

namespace services
{
#ifdef EMIL_USE_MBEDTLS
    struct CertificateAndPrivateKey
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<512> certificate;
        std::array<uint8_t, 121> privateKey;
    };

    CertificateAndPrivateKey GenerateRootCertificate(hal::SynchronousRandomDataGenerator& randomDataGenerator);
    CertificateAndPrivateKey GenerateDeviceCertificate(const EcSecP256r1PrivateKey& issuerKey, hal::SynchronousRandomDataGenerator& randomDataGenerator);
#endif

    class EchoPolicyDiffieHellman
        : private EchoInitializationObserver
        , private EchoPolicy
        , private sesame_security::DiffieHellmanKeyEstablishment
        , private sesame_security::DiffieHellmanKeyEstablishmentProxy
    {
    public:
        struct Crypto
        {
            infra::CreatorBase<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchange;
            EcSecP256r1DsaSigner& signer;
            infra::CreatorBase<EcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>& verifier;
            HmacDrbgSha256& keyExpander;
        };

        struct KeyMaterial
        {
            infra::ConstByteRange dsaCertificate;
            infra::ConstByteRange dsaCertificatePrivateKey;
            infra::ConstByteRange rootCaCertificate;
        };

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif

        EchoPolicyDiffieHellman(const Crypto& crypto, EchoWithPolicy& echo, EchoInitialization& echoInitialization, SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator);

    private:
        // Implementation of SesameObserver
        void Initialized() override;

        // Implementation of EchoPolicy
        void RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest) override;
        void GrantingSend(ServiceProxy& proxy) override;

        virtual void KeyExchangeSuccessful();
        virtual void KeyExchangeFailed();

    private:
        // Implementation of DiffieHellmanKeyEstablishment
        void Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS) override;
        void PresentCertificate(infra::ConstByteRange otherDsaCertificate) override;

        void ReQueueWaitingProxies();

    private:
        SesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::ConstByteRange dsaCertificate;
        infra::ConstByteRange rootCaCertificate;

        infra::Function<void(ServiceProxy& proxy)> onRequest;

        bool initializingKeys = true;
        bool busy = false;
        std::optional<std::pair<std::array<uint8_t, 16>, std::array<uint8_t, 16>>> nextKeyPair;
        infra::IntrusiveList<ServiceProxy> waitingProxies;

        infra::CreatorBase<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchangeCreator;
        std::optional<infra::ProxyCreator<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>> keyExchange;
        EcSecP256r1DsaSigner& signer;
        infra::CreatorBase<EcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>& verifierCreator;
        std::optional<infra::ProxyCreator<EcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>> verifier;
        HmacDrbgSha256& keyExpander;
    };

#ifdef EMIL_USE_MBEDTLS
    struct EchoPolicyDiffieHellman::WithCryptoMbedTls
        : public EchoPolicyDiffieHellman
    {
        WithCryptoMbedTls(EchoWithPolicy& echo, EchoInitialization& echoInitialization, SesameSecured& secured, const EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        infra::Creator<EcSecP256r1DiffieHellman, EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<EcSecP256r1DsaVerifier, EcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
