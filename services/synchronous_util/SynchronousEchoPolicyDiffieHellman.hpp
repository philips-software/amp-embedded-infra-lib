#ifndef SERVICES_SYNCHRONOUS_ECHO_POLICY_DIFFIE_HELLMAN_HPP
#define SERVICES_SYNCHRONOUS_ECHO_POLICY_DIFFIE_HELLMAN_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "services/synchronous_util/SynchronousSesameCrypto.hpp"
#include "services/util/EchoOnSesame.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/synchronous_util/SynchronousSesameCryptoMbedTls.hpp"
#endif
#include "services/synchronous_util/SynchronousSesameSecured.hpp"

namespace services
{
#ifdef EMIL_USE_MBEDTLS
    struct CertificateAndPrivateKey
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<512> certificate;
        EcSecP256r1PrivateKey::DerEncoded privateKey;
    };

    CertificateAndPrivateKey GenerateRootCertificate(hal::SynchronousRandomDataGenerator& randomDataGenerator);
    CertificateAndPrivateKey GenerateDeviceCertificate(const EcSecP256r1PrivateKey& issuerKey, hal::SynchronousRandomDataGenerator& randomDataGenerator);
#endif

    class SynchronousEchoPolicyDiffieHellman
        : private EchoInitializationObserver
        , private EchoPolicy
        , private sesame_security::DiffieHellmanKeyEstablishment
        , private sesame_security::DiffieHellmanKeyEstablishmentProxy
    {
    public:
        struct Crypto
        {
            infra::CreatorBase<SynchronousEcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchange;
            SynchronousEcSecP256r1DsaSigner& signer;
            infra::CreatorBase<SynchronousEcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>& verifier;
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

        SynchronousEchoPolicyDiffieHellman(const Crypto& crypto, Echo& echo, EchoInitialization& echoInitialization, SynchronousSesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator);

    private:
        // Implementation of EchoInitializationObserver
        void Reset() override;
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
        SynchronousSesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::ConstByteRange dsaCertificate;
        infra::ConstByteRange rootCaCertificate;

        infra::Function<void(ServiceProxy& proxy)> onRequest;

        bool initializingKeys = true;
        bool busy = false;
        std::optional<std::pair<SynchronousSesameSecured::KeyType, SynchronousSesameSecured::IvType>> nextKeyPair;
        infra::IntrusiveList<ServiceProxy> waitingProxies;

        infra::CreatorBase<SynchronousEcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchangeCreator;
        std::optional<infra::ProxyCreator<SynchronousEcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>> keyExchange;
        SynchronousEcSecP256r1DsaSigner& signer;
        infra::CreatorBase<SynchronousEcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>& verifierCreator;
        std::optional<infra::ProxyCreator<SynchronousEcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>> verifier;
        HmacDrbgSha256& keyExpander;
    };

#ifdef EMIL_USE_MBEDTLS
    struct SynchronousEchoPolicyDiffieHellman::WithCryptoMbedTls
        : public SynchronousEchoPolicyDiffieHellman
    {
        WithCryptoMbedTls(Echo& echo, EchoInitialization& echoInitialization, SynchronousSesameSecured& secured, const SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        infra::Creator<SynchronousEcSecP256r1DiffieHellman, SynchronousEcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        SynchronousEcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<SynchronousEcSecP256r1DsaVerifier, SynchronousEcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
