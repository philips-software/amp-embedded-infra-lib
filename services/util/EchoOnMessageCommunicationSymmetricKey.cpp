#include "services/util/EchoOnMessageCommunicationSymmetricKey.hpp"

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

    EchoOnMessageCommunicationSymmetricKey::EchoOnMessageCommunicationSymmetricKey(MessageCommunicationSecured& secured, MethodSerializerFactory& serializerFactory, hal::SynchronousRandomDataGenerator& randomDataGenerator, const EchoErrorPolicy& errorPolicy)
        : EchoOnMessageCommunication(secured, serializerFactory, errorPolicy)
        , SymmetricKeyEstablishment(static_cast<services::Echo&>(*this))
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
                SymmetricKeyEstablishmentProxy::ActivateNewKeyMaterial(infra::MakeRange(key), infra::MakeRange(iv));
                secured.SetNextSendKey(key, iv);

                initializingSending = false;
                ReQueueWaitingProxies();
            });
    }

    void EchoOnMessageCommunicationSymmetricKey::ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv)
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
