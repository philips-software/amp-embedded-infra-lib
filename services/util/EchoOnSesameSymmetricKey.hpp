#ifndef SERVICES_ECHO_ON_SESAME_SYMMETRIC_KEY_HPP
#define SERVICES_ECHO_ON_SESAME_SYMMETRIC_KEY_HPP

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
        EchoOnSesameSymmetricKey(SesameSecured& secured, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;

        // Implementation of SesameObserver
        void Initialized() override;

    protected:
        // Implementation of EchoOnStreams
        infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy) override;

    private:
        // Implementation of SymmetricKeyEstablishment
        void ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv) override;

        void ReQueueWaitingProxies();

    private:
        SesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        bool initializingSending = true;
        infra::IntrusiveList<ServiceProxy> waitingProxies;
        infra::Optional<std::pair<std::array<uint8_t, 16>, std::array<uint8_t, 16>>> nextKeyPair;
    };
}

#endif
