#include "services/util/EchoPolicySymmetricKey.hpp"

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

    EchoPolicySymmetricKey::EchoPolicySymmetricKey(EchoWithPolicy& echo, EchoInitialization& echoInitialization, SesameSecured& secured, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoInitializationObserver(echoInitialization)
        , SymmetricKeyEstablishment(echo)
        , SymmetricKeyEstablishmentProxy(echo)
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
    {
        echo.SetPolicy(*this);
    }

    void EchoPolicySymmetricKey::Initialized()
    {
        initializingSending = true;

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

    void EchoPolicySymmetricKey::RequestSend(ServiceProxy& serviceProxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest)
    {
        this->onRequest = onRequest;

        if (initializingSending && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            onRequest(serviceProxy);
    }

    void EchoPolicySymmetricKey::GrantingSend(ServiceProxy& proxy)
    {
        if (nextKeyPair && &proxy != this)
        {
            secured.SetSendKey(nextKeyPair->first, nextKeyPair->second);
            nextKeyPair.reset();
        }
    }

    void EchoPolicySymmetricKey::ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv)
    {
        secured.SetReceiveKey(Convert<16>(key), Convert<16>(iv));
        MethodDone();
    }

    void EchoPolicySymmetricKey::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            onRequest(proxy);
        }
    }
}
