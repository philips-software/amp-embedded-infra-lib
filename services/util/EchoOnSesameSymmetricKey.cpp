#include "services/util/EchoOnSesameSymmetricKey.hpp"

namespace services
{
    namespace
    {
        template<std::size_t Size>
        std::array<uint8_t, Size> Convert(infra::ConstByteRange value)
        {
            really_assert(Size == value.size());
            std::array<uint8_t, Size> result;
            infra::Copy(value, infra::MakeRange(result));
            return result;
        }
    }

    EchoOnSesameSymmetricKey::EchoOnSesameSymmetricKey(SesameSecured& secured, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnSesame(secured, serializerFactory, errorPolicy)
        , SymmetricKeyEstablishment(static_cast<services::Echo&>(*this))
        , SymmetricKeyEstablishmentProxy(static_cast<services::Echo&>(*this))
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
    {}

    void EchoOnSesameSymmetricKey::RequestSend(ServiceProxy& serviceProxy)
    {
        if (initializingSending && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            EchoOnSesame::RequestSend(serviceProxy);
    }

    void EchoOnSesameSymmetricKey::Initialized()
    {
        initializingSending = true;
        EchoOnSesame::Initialized();

        SymmetricKeyEstablishmentProxy::RequestSend([this]()
            {
                auto key = randomDataGenerator.GenerateRandomData<SesameSecured::KeyType>();
                auto iv = randomDataGenerator.GenerateRandomData<SesameSecured::IvType>();
                SymmetricKeyEstablishmentProxy::ActivateNewKeyMaterial(infra::MakeRange(key), infra::MakeRange(iv));
                nextKeyPair = { key, iv };

                initializingSending = false;
                ReQueueWaitingProxies();
            });
    }

    infra::SharedPtr<MethodSerializer> EchoOnSesameSymmetricKey::GrantSend(ServiceProxy& proxy)
    {
        if (nextKeyPair && &proxy != this)
        {
            secured.SetSendKey(nextKeyPair->first, nextKeyPair->second);
            nextKeyPair = infra::none;
        }

        return EchoOnStreams::GrantSend(proxy);
    }

    void EchoOnSesameSymmetricKey::ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv)
    {
        secured.SetReceiveKey(Convert<16>(key), Convert<16>(iv));
        MethodDone();
    }

    void EchoOnSesameSymmetricKey::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            RequestSend(proxy);
        }
    }
}
