#ifndef SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP
#define SERVICES_ECHO_ON_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameSecured.hpp"

namespace services
{
    class EchoOnSesameSymmetricKey
        : public EchoOnSesame
        , private sesame_security::SymmetricKeyEstablishment
        , private sesame_security::SymmetricKeyEstablishmentProxy
    {
    public:
        EchoOnSesameSymmetricKey(SesameSecured& secured, MethodSerializerFactory& serializerFactory, hal::SynchronousRandomDataGenerator& randomDataGenerator, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of SesameObserver
        void Initialized() override;

    private:
        // Implementation of SymmetricKeyEstablishment
        void ActivateNewKeyMaterial(const infra::BoundedVector<uint8_t>& key, const infra::BoundedVector<uint8_t>& iv) override;

        void ReQueueWaitingProxies();

    private:
        SesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        bool initializingSending = true;
        infra::IntrusiveList<ServiceProxy> waitingProxies;
    };
}

#endif
