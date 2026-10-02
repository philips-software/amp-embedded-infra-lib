#include "services/synchronous_util/SynchronousEchoPolicySymmetricKey.hpp"

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

    SynchronousEchoPolicySymmetricKey::SynchronousEchoPolicySymmetricKey(Echo& echo, EchoInitialization& echoInitialization, SynchronousSesameSecured& secured, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoInitializationObserver(echoInitialization)
        , SymmetricKeyEstablishment(echo)
        , SymmetricKeyEstablishmentProxy(echo)
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
    {
        echo.SetPolicy(*this);
    }

    void SynchronousEchoPolicySymmetricKey::Reset()
    {
        initializingSending = true;
    }

    void SynchronousEchoPolicySymmetricKey::Initialized()
    {
        initializingSending = true;

        SymmetricKeyEstablishmentProxy::RequestSend([this]()
            {
                auto key = randomDataGenerator.GenerateRandomData<SynchronousSesameSecured::KeyType>();
                auto iv = randomDataGenerator.GenerateRandomData<SynchronousSesameSecured::IvType>();
                SymmetricKeyEstablishmentProxy::ActivateNewKeyMaterial(infra::MakeRange(key), infra::MakeRange(iv));
                nextKeyPair = { key, iv };

                initializingSending = false;
                ReQueueWaitingProxies();
            });
    }

    void SynchronousEchoPolicySymmetricKey::RequestSend(ServiceProxy& serviceProxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest)
    {
        this->onRequest = onRequest;

        if (initializingSending && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            onRequest(serviceProxy);
    }

    void SynchronousEchoPolicySymmetricKey::GrantingSend(ServiceProxy& proxy)
    {
        if (nextKeyPair && &proxy != this)
        {
            secured.SetSendKey(nextKeyPair->first, nextKeyPair->second);
            nextKeyPair.reset();
        }
    }

    void SynchronousEchoPolicySymmetricKey::ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv)
    {
        secured.SetReceiveKey(Convert<16>(key), Convert<12>(iv));
        MethodDone();
    }

    void SynchronousEchoPolicySymmetricKey::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            onRequest(proxy);
        }
    }
}
