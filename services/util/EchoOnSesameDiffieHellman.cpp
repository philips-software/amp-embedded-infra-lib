#include "services/util/EchoOnSesameDiffieHellman.hpp"

namespace services
{
    EchoOnSesameDiffieHellman::EchoOnSesameDiffieHellman(const Crypto& crypto, SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnSesame(secured, serializerFactory, errorPolicy)
        , DiffieHellmanKeyEstablishment(static_cast<services::Echo&>(*this))
        , DiffieHellmanKeyEstablishmentProxy(static_cast<services::Echo&>(*this))
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
        , dsaCertificate(dsaCertificate)
        , rootCaCertificate(rootCaCertificate)
        , keyExchangeCreator(crypto.keyExchange)
        , signer(crypto.signer)
        , verifierCreator(crypto.verifier)
        , keyExpander(crypto.keyExpander)
    {}

    void EchoOnSesameDiffieHellman::RequestSend(ServiceProxy& serviceProxy)
    {
        if (initializingKeys && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            EchoOnSesame::RequestSend(serviceProxy);
    }

    void EchoOnSesameDiffieHellman::Initialized()
    {
        initializingKeys = true;
        EchoOnSesame::Initialized();

        keyExchange.Emplace(keyExchangeCreator, randomDataGenerator);

        DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
            {
                DiffieHellmanKeyEstablishmentProxy::PresentCertificate(dsaCertificate);

                DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
                    {
                        auto encodedDhPublicKey = (*keyExchange)->PublicKey();
                        auto [r, s] = signer.Sign(encodedDhPublicKey);

                        sentExchange = true;
                        DiffieHellmanKeyEstablishmentProxy::Exchange(encodedDhPublicKey, r, s);
                    });
            });
    }

    infra::SharedPtr<MethodSerializer> EchoOnSesameDiffieHellman::GrantSend(ServiceProxy& proxy)
    {
        if (nextKeyPair && &proxy != this)
        {
            secured.SetSendKey(nextKeyPair->first, nextKeyPair->second);
            nextKeyPair = infra::none;
        }

        return EchoOnSesame::GrantSend(proxy);
    }

    template<std::size_t Size>
    std::array<uint8_t, Size> Middle(infra::ConstByteRange range, std::size_t start)
    {
        std::array<uint8_t, Size> result;
        infra::Copy(infra::Head(infra::DiscardHead(range, start), Size), infra::MakeRange(result));
        return result;
    }

    void EchoOnSesameDiffieHellman::Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
    {
        if (verifier == infra::none || !(*verifier)->Verify(otherPublicKey, signatureR, signatureS))
            return;

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

        initializingKeys = false;
        ReQueueWaitingProxies();
        MethodDone();
    }

    void EchoOnSesameDiffieHellman::PresentCertificate(infra::BoundedConstString otherDsaCertificate)
    {
        verifier.Emplace(verifierCreator, otherDsaCertificate, rootCaCertificate);

        MethodDone();
    }

    void EchoOnSesameDiffieHellman::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            RequestSend(proxy);
        }
    }

#ifdef EMIL_USE_MBEDTLS
    EchoOnSesameDiffieHellman::WithCryptoMbedTls::WithCryptoMbedTls(SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString dsaCertificatePrivateKey, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnSesameDiffieHellman(Crypto{ keyExchange, signer, verifier, keyExpander }, secured, dsaCertificate, rootCaCertificate, randomDataGenerator, serializerFactory, errorPolicy)
        , signer(dsaCertificatePrivateKey, randomDataGenerator)
    {}
#endif
}
