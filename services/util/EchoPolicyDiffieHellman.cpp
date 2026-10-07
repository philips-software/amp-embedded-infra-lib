#include "services/util/EchoPolicyDiffieHellman.hpp"
#include <algorithm>

namespace services
{
    namespace
    {
        template<std::size_t Size>
        std::array<uint8_t, Size> Middle(infra::ConstByteRange range, std::size_t start)
        {
            std::array<uint8_t, Size> result;
            infra::Copy(infra::Head(infra::DiscardHead(range, start), Size), infra::MakeRange(result));
            return result;
        }
    }

    EchoPolicyDiffieHellman::EchoPolicyDiffieHellman(const Crypto& crypto, Echo& echo, EchoInitialization& echoInitialization, SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)
        : EchoInitializationObserver(echoInitialization)
        , DiffieHellmanKeyEstablishment(echo)
        , DiffieHellmanKeyEstablishmentProxy(echo)
        , secured(secured)
        , dsaCertificate(dsaCertificate)
        , rootCaCertificate(rootCaCertificate)
        , keyExchange(crypto.keyExchange)
        , signer(crypto.signer)
        , verifier(crypto.verifier)
        , keyExpander(crypto.keyExpander)
    {
        echo.SetPolicy(*this);
        GenerateKeyPair();
    }

    void EchoPolicyDiffieHellman::Reset()
    {
        ++epoch;
        busy = false;
        requestSendPending = false;
        initializingKeys = true;

        if (keyPairUsed)
            GenerateKeyPair();
    }

    void EchoPolicyDiffieHellman::Initialized()
    {
        if (requestSendPending)
            DiffieHellmanKeyEstablishmentProxy::CancelRequestSend();

        ++epoch;
        initializingKeys = true;
        requestSendPending = false;
        certificateSent = false;
        exchangeRequested = false;
        exchangeSent = false;
        sharedSecretComputed = false;
        otherCertificateValid = false;
        peerPublicKeyVerified = false;
        nextKeyPair.reset();

        if (keyPairUsed)
            GenerateKeyPair();

        busy = true;
        requestSendPending = true;
        DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
            {
                requestSendPending = false;
                DiffieHellmanKeyEstablishmentProxy::PresentCertificate(dsaCertificate);
                certificateSent = true;
                TrySendExchange();
            });
    }

    void EchoPolicyDiffieHellman::RequestSend(ServiceProxy& serviceProxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest)
    {
        this->onRequest = onRequest;

        if (initializingKeys && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            onRequest(serviceProxy);
    }

    void EchoPolicyDiffieHellman::GrantingSend(ServiceProxy& proxy)
    {
        if (nextKeyPair && &proxy != this)
        {
            secured.SetSendKey(nextKeyPair->first, nextKeyPair->second);
            nextKeyPair.reset();
        }
    }

    void EchoPolicyDiffieHellman::KeyExchangeSuccessful()
    {}

    void EchoPolicyDiffieHellman::KeyExchangeFailed()
    {}

    void EchoPolicyDiffieHellman::Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
    {
        if (!otherCertificateValid || otherPublicKey.size() != peerPublicKey.size())
        {
            KeyExchangeFailed();
            return;
        }

        infra::Copy(otherPublicKey, infra::MakeRange(peerPublicKey));

        verifier.Verify(otherPublicKey, signatureR, signatureS, [this, currentEpoch = epoch](bool valid)
            {
                if (currentEpoch != epoch)
                    return;

                if (!valid)
                {
                    KeyExchangeFailed();
                    return;
                }

                peerPublicKeyVerified = true;
                if (ownPublicKey != std::nullopt)
                    ComputeSharedSecret();
            });
    }

    void EchoPolicyDiffieHellman::PresentCertificate(infra::ConstByteRange otherDsaCertificate)
    {
        verifier.VerifyCertificate(otherDsaCertificate, rootCaCertificate, [this, currentEpoch = epoch](bool valid)
            {
                if (currentEpoch != epoch)
                    return;

                otherCertificateValid = valid;
                MethodDone();
            });
    }

    void EchoPolicyDiffieHellman::GenerateKeyPair()
    {
        ++keyPairEpoch;
        keyPairUsed = false;
        ownPublicKey.reset();
        ownSignature.reset();

        keyExchange.GenerateKeyPair([this, currentKeyPairEpoch = keyPairEpoch](const std::array<uint8_t, 65>& publicKey)
            {
                if (currentKeyPairEpoch == keyPairEpoch)
                    KeyPairGenerated(publicKey);
            });
    }

    void EchoPolicyDiffieHellman::KeyPairGenerated(const std::array<uint8_t, 65>& publicKey)
    {
        ownPublicKey = publicKey;

        signer.Sign(*ownPublicKey, [this, currentKeyPairEpoch = keyPairEpoch](const std::array<uint8_t, 32>& r, const std::array<uint8_t, 32>& s)
            {
                if (currentKeyPairEpoch != keyPairEpoch)
                    return;

                ownSignature.emplace(r, s);
                TrySendExchange();
            });

        if (peerPublicKeyVerified)
            ComputeSharedSecret();
    }

    void EchoPolicyDiffieHellman::TrySendExchange()
    {
        if (!certificateSent || exchangeRequested || ownSignature == std::nullopt)
            return;

        exchangeRequested = true;
        keyPairUsed = true;
        requestSendPending = true;
        DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
            {
                requestSendPending = false;
                DiffieHellmanKeyEstablishmentProxy::Exchange(*ownPublicKey, ownSignature->first, ownSignature->second);
                exchangeSent = true;
                busy = false;
                ReQueueWaitingProxies();
                GenerateNextKeyPairWhenDone();
            });
    }

    void EchoPolicyDiffieHellman::ComputeSharedSecret()
    {
        peerPublicKeyVerified = false;
        keyPairUsed = true;

        keyExchange.SharedSecret(peerPublicKey, [this, currentEpoch = epoch](const std::array<uint8_t, 32>& sharedSecret)
            {
                if (currentEpoch == epoch)
                    SharedSecretComputed(sharedSecret);
            });
    }

    void EchoPolicyDiffieHellman::SharedSecretComputed(const std::array<uint8_t, 32>& sharedSecret)
    {
        int swap = std::lexicographical_compare(ownPublicKey->begin(), ownPublicKey->end(), peerPublicKey.begin(), peerPublicKey.end()) ? 28 : 0;

        std::array<uint8_t, 64> expandedMaterial{};
        keyExpander.Expand(sharedSecret, expandedMaterial);

        auto key = Middle<16>(expandedMaterial, 0 + swap);
        auto iv = Middle<12>(expandedMaterial, 16 + swap);
        auto otherKey = Middle<16>(expandedMaterial, 28 - swap);
        auto otherIv = Middle<12>(expandedMaterial, 44 - swap);

        nextKeyPair = { key, iv };
        secured.SetReceiveKey(otherKey, otherIv);

        KeyExchangeSuccessful();

        initializingKeys = false;
        ReQueueWaitingProxies();
        MethodDone();

        sharedSecretComputed = true;
        GenerateNextKeyPairWhenDone();
    }

    void EchoPolicyDiffieHellman::GenerateNextKeyPairWhenDone()
    {
        if (exchangeSent && sharedSecretComputed)
            GenerateKeyPair();
    }

    void EchoPolicyDiffieHellman::ReQueueWaitingProxies()
    {
        while (!initializingKeys && !busy && !waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            onRequest(proxy);
        }
    }
}
