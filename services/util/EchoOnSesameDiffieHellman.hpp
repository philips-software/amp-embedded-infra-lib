#ifndef SERVICES_ECHO_ON_SESAME_DIFFIE_HELLMAN_HPP
#define SERVICES_ECHO_ON_SESAME_DIFFIE_HELLMAN_HPP

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
    std::pair<std::array<uint8_t, 121>, infra::BoundedVector<uint8_t>::WithMaxSize<512>> GenerateRootCertificate(hal::SynchronousRandomDataGenerator& randomDataGenerator);
    std::pair<std::array<uint8_t, 121>, infra::BoundedVector<uint8_t>::WithMaxSize<512>> GenerateDeviceCertificate(const EcSecP256r1PrivateKey& issuerKey, hal::SynchronousRandomDataGenerator& randomDataGenerator);
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
            infra::CreatorBase<EcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>& verifier;
            HmacDrbgSha256& keyExpander;
        };

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif

        EchoOnSesameDiffieHellman(const Crypto& crypto, SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of SesameObserver
        void Initialized() override;

    protected:
        // Implementation of EchoOnStreams
        infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy) override;

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

        bool initializingKeys = true;
        bool sentExchange = false;
        infra::Optional<std::pair<std::array<uint8_t, 16>, std::array<uint8_t, 16>>> nextKeyPair;
        infra::IntrusiveList<ServiceProxy> waitingProxies;

        infra::CreatorBase<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>& keyExchangeCreator;
        infra::Optional<infra::ProxyCreator<EcSecP256r1DiffieHellman, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)>> keyExchange;
        EcSecP256r1DsaSigner& signer;
        infra::CreatorBase<EcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>& verifierCreator;
        infra::Optional<infra::ProxyCreator<EcSecP256r1DsaVerifier, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)>> verifier;
        HmacDrbgSha256& keyExpander;
    };

#ifdef EMIL_USE_MBEDTLS
    struct EchoOnSesameDiffieHellman::WithCryptoMbedTls
        : public EchoOnSesameDiffieHellman
    {
        WithCryptoMbedTls(SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange dsaCertificatePrivateKey, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        infra::Creator<EcSecP256r1DiffieHellman, EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<EcSecP256r1DsaVerifier, EcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
