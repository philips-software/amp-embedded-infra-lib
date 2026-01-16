#include "services/util/EchoPolicyDiffieHellman.hpp"

namespace services
{
#ifdef EMIL_USE_MBEDTLS
    CertificateAndPrivateKey GenerateRootCertificate(hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        services::EcSecP256r1PrivateKey rootPrivateKey{ randomDataGenerator };
        services::EcSecP256r1Certificate rootCertificate{ rootPrivateKey, "CN=Root", rootPrivateKey, "CN=Root", randomDataGenerator };

        return { rootCertificate.Der(), rootPrivateKey.Der() };
    }

    CertificateAndPrivateKey GenerateDeviceCertificate(const EcSecP256r1PrivateKey& issuerKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        services::EcSecP256r1PrivateKey devicePrivateKey{ randomDataGenerator };
        services::EcSecP256r1Certificate deviceCertificate{ devicePrivateKey, "CN=Device", issuerKey, "CN=Root", randomDataGenerator };

        return { deviceCertificate.Der(), devicePrivateKey.Der() };
    }
#endif

    EchoPolicyDiffieHellman::EchoPolicyDiffieHellman(const Crypto& crypto, EchoWithPolicy& echo, EchoInitialization& echoInitialization, SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoInitializationObserver(echoInitialization)
        , DiffieHellmanKeyEstablishment(echo)
        , DiffieHellmanKeyEstablishmentProxy(echo)
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
        , dsaCertificate(dsaCertificate)
        , rootCaCertificate(rootCaCertificate)
        , keyExchangeCreator(crypto.keyExchange)
        , signer(crypto.signer)
        , verifierCreator(crypto.verifier)
        , keyExpander(crypto.keyExpander)
    {
        echo.SetPolicy(*this);
    }

    void EchoPolicyDiffieHellman::Initialized()
    {
        if (busy)
            DiffieHellmanKeyEstablishmentProxy::Rpc().CancelRequestSend(*this);

        initializingKeys = true;
        nextKeyPair.reset();
        verifier.reset();

        keyExchange.emplace(keyExchangeCreator, randomDataGenerator);

        busy = true;
        DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
            {
                DiffieHellmanKeyEstablishmentProxy::PresentCertificate(dsaCertificate);

                DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
                    {
                        auto encodedDhPublicKey = (*keyExchange)->PublicKey();
                        auto [r, s] = signer.Sign(encodedDhPublicKey);

                        DiffieHellmanKeyEstablishmentProxy::Exchange(encodedDhPublicKey, r, s);
                        busy = false;
                    });
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

    template<std::size_t Size>
    std::array<uint8_t, Size> Middle(infra::ConstByteRange range, std::size_t start)
    {
        std::array<uint8_t, Size> result;
        infra::Copy(infra::Head(infra::DiscardHead(range, start), Size), infra::MakeRange(result));
        return result;
    }

    void EchoPolicyDiffieHellman::Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
    {
        if (verifier == std::nullopt || !(*verifier)->Verify(otherPublicKey, signatureR, signatureS))
        {
            KeyExchangeFailed();
            return;
        }

        auto sharedSecret = (*keyExchange)->SharedSecret(otherPublicKey);
        auto publicKey = (*keyExchange)->PublicKey();

        int swap = std::lexicographical_compare(publicKey.begin(), publicKey.end(), otherPublicKey.begin(), otherPublicKey.end()) ? 32 : 0;

        std::array<uint8_t, 64> expandedMaterial{};
        keyExpander.Expand(sharedSecret, expandedMaterial);

        std::array<uint8_t, 16> key = Middle<16>(expandedMaterial, 0 + swap);
        std::array<uint8_t, 16> iv = Middle<16>(expandedMaterial, 16 + swap);
        std::array<uint8_t, 16> otherKey = Middle<16>(expandedMaterial, 32 - swap);
        std::array<uint8_t, 16> otherIv = Middle<16>(expandedMaterial, 48 - swap);

        nextKeyPair = { key, iv };
        secured.SetReceiveKey(otherKey, otherIv);

        KeyExchangeSuccessful();

        initializingKeys = false;
        ReQueueWaitingProxies();
        MethodDone();
    }

    void EchoPolicyDiffieHellman::PresentCertificate(infra::ConstByteRange otherDsaCertificate)
    {
        verifier.emplace(verifierCreator, otherDsaCertificate, rootCaCertificate);

        MethodDone();
    }

    void EchoPolicyDiffieHellman::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            onRequest(proxy);
        }
    }

#ifdef EMIL_USE_MBEDTLS
    EchoPolicyDiffieHellman::WithCryptoMbedTls::WithCryptoMbedTls(EchoWithPolicy& echo, EchoInitialization& echoInitialization, SesameSecured& secured, const EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoPolicyDiffieHellman(Crypto{ keyExchange, signer, verifier, keyExpander }, echo, echoInitialization, secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
        , signer(keyMaterial.dsaCertificatePrivateKey, randomDataGenerator)
    {}
#endif
}
