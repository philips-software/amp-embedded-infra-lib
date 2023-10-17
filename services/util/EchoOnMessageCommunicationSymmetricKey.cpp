#include "services/util/EchoOnMessageCommunicationSymmetricKey.hpp"

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

    EchoOnMessageCommunicationSymmetricKey::EchoOnMessageCommunicationSymmetricKey(MessageCommunicationSecured& secured, MethodDeserializerFactory& deserializerFactory, hal::SynchronousRandomDataGenerator& randomDataGenerator, EchoErrorPolicy& errorPolicy)
        : EchoOnMessageCommunication(secured, errorPolicy)
        , SymmetricKeyEstablishment(static_cast<services::Echo&>(*this), deserializerFactory)
        , SymmetricKeyEstablishmentProxy(static_cast<services::Echo&>(*this))
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
    {}

    void EchoOnMessageCommunicationSymmetricKey::RequestSend(ServiceProxy& serviceProxy)
    {
        if (initializingSending && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            EchoOnMessageCommunication::RequestSend(serviceProxy);
    }

    void EchoOnMessageCommunicationSymmetricKey::Initialized()
    {
        initializingSending = true;
        EchoOnMessageCommunication::Initialized();

        SymmetricKeyEstablishmentProxy::RequestSend([this]()
            {
                auto key = randomDataGenerator.GenerateRandomData<MessageCommunicationSecured::KeyType>();
                auto iv = randomDataGenerator.GenerateRandomData<MessageCommunicationSecured::IvType>();
                SymmetricKeyEstablishmentProxy::ActivateNewKeyMaterial(Convert(key), Convert(iv));
                secured.SetNextSendKey(key, iv);

                initializingSending = false;
                ReQueueWaitingProxies();
            });
    }

    void EchoOnMessageCommunicationSymmetricKey::ActivateNewKeyMaterial(const infra::BoundedVector<uint8_t>& key, const infra::BoundedVector<uint8_t>& iv)
    {
        secured.SetReceiveKey(Convert<16>(key), Convert<16>(iv));
        MethodDone();
    }

    void EchoOnMessageCommunicationSymmetricKey::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            RequestSend(proxy);
        }
    }
}
