#ifndef SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP
#define SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP

#include "generated/echo/MessageCommunicationSecurity.pb.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "services/util/EchoOnMessageCommunication.hpp"
#include "services/util/MessageCommunicationSecured.hpp"

namespace services
{
    class EchoOnMessageCommunicationSymmetricKey
        : public EchoOnMessageCommunication
        , private message_communication_security::SymmetricKeyEstablishment
        , private message_communication_security::SymmetricKeyEstablishmentProxy
    {
    public:
        EchoOnMessageCommunicationSymmetricKey(MessageCommunicationSecured& secured, MethodSerializerFactory& serializerFactory, hal::SynchronousRandomDataGenerator& randomDataGenerator, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of MessageCommunicationObserver
        void Initialized() override;

    private:
        // Implementation of SymmetricKeyEstablishment
        void ActivateNewKeyMaterial(const infra::BoundedVector<uint8_t>& key, const infra::BoundedVector<uint8_t>& iv) override;

        void ReQueueWaitingProxies();

    private:
        MessageCommunicationSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        bool initializingSending = true;
        infra::IntrusiveList<ServiceProxy> waitingProxies;
    };
}

#endif
