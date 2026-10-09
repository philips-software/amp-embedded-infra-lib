#ifndef SERVICES_ECHO_POLICY_DIFFIE_HELLMAN_HPP
#define SERVICES_ECHO_POLICY_DIFFIE_HELLMAN_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "services/synchronous_util/SynchronousSesameCrypto.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameCrypto.hpp"
#include "services/util/SesameSecured.hpp"
#include <optional>

namespace services
{
    class EchoPolicyDiffieHellman
        : private EchoInitializationObserver
        , private EchoPolicy
        , private sesame_security::DiffieHellmanKeyEstablishment
        , private sesame_security::DiffieHellmanKeyEstablishmentProxy
    {
    public:
        struct Crypto
        {
            EcSecP256r1DiffieHellman& keyExchange;
            EcSecP256r1DsaSigner& signer;
            EcSecP256r1DsaVerifier& verifier;
            HmacDrbgSha256& keyExpander;
        };

        EchoPolicyDiffieHellman(const Crypto& crypto, Echo& echo, EchoInitialization& echoInitialization, SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate);

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

        void KeyPairGenerated(const std::array<uint8_t, 65>& publicKey);
        void TrySendExchange();
        void ComputeSharedSecret();
        void SharedSecretComputed(const std::array<uint8_t, 32>& sharedSecret);
        void ReQueueWaitingProxies();

    private:
        SesameSecured& secured;
        infra::ConstByteRange dsaCertificate;
        infra::ConstByteRange rootCaCertificate;
        EcSecP256r1DiffieHellman& keyExchange;
        EcSecP256r1DsaSigner& signer;
        EcSecP256r1DsaVerifier& verifier;
        HmacDrbgSha256& keyExpander;

        infra::Function<void(ServiceProxy& proxy)> onRequest;

        uint32_t epoch = 0;
        bool initializingKeys = true;
        bool busy = false;
        bool requestSendPending = false;
        bool certificateSent = false;
        bool otherCertificateValid = false;
        bool peerPublicKeyVerified = false;
        std::optional<std::array<uint8_t, 65>> ownPublicKey;
        std::optional<std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>>> ownSignature;
        std::array<uint8_t, 65> peerPublicKey{};
        std::optional<std::pair<SesameSecured::KeyType, SesameSecured::IvType>> nextKeyPair;
        infra::IntrusiveList<ServiceProxy> waitingProxies;
    };
}

#endif
