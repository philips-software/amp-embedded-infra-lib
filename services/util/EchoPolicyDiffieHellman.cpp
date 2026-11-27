#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "infra/util/ReallyAssert.hpp"

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

    EchoPolicyDiffieHellman::EchoPolicyDiffieHellman(const Crypto& crypto, Echo& echo, EchoInitialization& echoInitialization, SesameSecured& secured, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)
        : EchoInitializationObserver(echoInitialization)
        , DiffieHellmanKeyEstablishment(echo)
        , DiffieHellmanKeyEstablishmentProxy(echo)
        , secured(secured)
        , dsaCertificate(dsaCertificate)
        , rootCaCertificate(rootCaCertificate)
        , keyExchangeCreator(crypto.keyExchange)
        , signer(crypto.signer)
        , verifierCreator(crypto.verifier)
        , keyExpander(crypto.keyExpander)
    {
        echo.SetPolicy(*this);
    }

    void EchoPolicyDiffieHellman::Reset()
    {
        busy = false;
        initializingKeys = true;

        while (!waitingProxies.empty())
            waitingProxies.front().CancelRequestSend();
    }

    void EchoPolicyDiffieHellman::Initialized()
    {
        if (busy)
            DiffieHellmanKeyEstablishmentProxy::CancelRequestSend();

        initializingKeys = true;
        if (nextKeyPair.has_value())
            Clean(infra::MakeByteRange(nextKeyPair.value()));
        nextKeyPair.reset();
        verifier.reset();

        keyExchange.emplace(keyExchangeCreator);

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
                        ReQueueWaitingProxies();
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
        if (nextKeyPair.has_value() && &proxy != this)
        {
            secured.SetSendKey(nextKeyPair.value().first, nextKeyPair.value().second);
            Clean(infra::MakeByteRange(nextKeyPair.value()));
            nextKeyPair.reset();
        }
    }

    bool EchoPolicyDiffieHellman::PendingRequestSend(ServiceProxy& proxy) const
    {
        return waitingProxies.has_element(proxy);
    }

    void EchoPolicyDiffieHellman::CancelRequestSend(ServiceProxy& proxy)
    {
        really_assert(PendingRequestSend(proxy));
        waitingProxies.erase(proxy);
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
            MethodDone();
            return;
        }

        auto sharedSecret = (*keyExchange)->SharedSecret(otherPublicKey);
        auto publicKey = (*keyExchange)->PublicKey();

        int swap = std::lexicographical_compare(publicKey.begin(), publicKey.end(), otherPublicKey.begin(), otherPublicKey.end()) ? 28 : 0;

        std::array<uint8_t, 64> expandedMaterial{};
        keyExpander.Expand(sharedSecret, expandedMaterial);
        Clean(infra::MakeRange(sharedSecret));

        auto key = Middle<16>(expandedMaterial, 0 + swap);
        auto iv = Middle<12>(expandedMaterial, 16 + swap);
        auto otherKey = Middle<16>(expandedMaterial, 28 - swap);
        auto otherIv = Middle<12>(expandedMaterial, 44 - swap);
        Clean(expandedMaterial);

        nextKeyPair = { key, iv };
        secured.SetReceiveKey(otherKey, otherIv);
        Clean(key);
        Clean(iv);
        Clean(otherKey);
        Clean(otherIv);

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
        while (!initializingKeys && !busy && !waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            onRequest(proxy);
        }
    }

    void EchoPolicyDiffieHellman::Clean(infra::ByteRange material) const
    {
        infra::MemoryRange<volatile uint8_t> materialRange = material;
        std::fill(materialRange.begin(), materialRange.end(), 0);
    }

#ifdef EMIL_USE_MBEDTLS
    detail::CryptoMbedTls::CryptoMbedTls(infra::ConstByteRange privateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : keyExchange([&randomDataGenerator](std::optional<EcSecP256r1DiffieHellmanMbedTls>& object)
              {
                  object.emplace(randomDataGenerator);
              })
        , signer(privateKey, randomDataGenerator)
    {
    }

    CryptoMbedTls::CryptoMbedTls(infra::ConstByteRange privateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : detail::CryptoMbedTls(privateKey, randomDataGenerator)
        , EchoPolicyDiffieHellman::Crypto{ detail::CryptoMbedTls::keyExchange, detail::CryptoMbedTls::signer, detail::CryptoMbedTls::verifier, detail::CryptoMbedTls::keyExpander, detail::CryptoMbedTls::encryptors }
    {
    }

    EchoPolicyDiffieHellman::WithCryptoMbedTls::WithCryptoMbedTls(Echo& echo, EchoInitialization& echoInitialization, SesameSecured& secured, const EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : CryptoMbedTls(keyMaterial.dsaCertificate, randomDataGenerator)
        , EchoPolicyDiffieHellman(static_cast<EchoPolicyDiffieHellman::Crypto&>(*this), echo, echoInitialization, secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate)
    {}
#endif
}
