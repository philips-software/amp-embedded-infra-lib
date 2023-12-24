#include "services/util/EchoOnSesameSymmetricKey.hpp"

namespace services
{
    namespace
    {
        template<std::size_t Size>
        std::array<uint8_t, Size> Convert(const infra::BoundedVector<uint8_t>& value)
        {
            really_assert(Size == value.size());
            std::array<uint8_t, Size> result;
            infra::Copy(infra::MakeRange(value), infra::MakeRange(result));
            return result;
        }

        template<std::size_t Size>
        infra::BoundedVector<uint8_t>::WithMaxSize<Size> Convert(const std::array<uint8_t, Size>& value)
        {
            return infra::BoundedVector<uint8_t>::WithMaxSize<Size>(value.begin(), value.end());
        }
    }

    EchoOnSesameSymmetricKey::EchoOnSesameSymmetricKey(SesameSecured& secured, MethodSerializerFactory& serializerFactory, hal::SynchronousRandomDataGenerator& randomDataGenerator, const EchoErrorPolicy& errorPolicy)
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
                SymmetricKeyEstablishmentProxy::ActivateNewKeyMaterial(Convert(key), Convert(iv));
                secured.SetNextSendKey(key, iv);

                initializingSending = false;
                ReQueueWaitingProxies();
            });
    }

    void EchoOnSesameSymmetricKey::ActivateNewKeyMaterial(const infra::BoundedVector<uint8_t>& key, const infra::BoundedVector<uint8_t>& iv)
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
