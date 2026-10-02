#ifndef SERVICES_SYNCHRONOUS_ECHO_POLICY_SYMMETRIC_KEY_HPP
#define SERVICES_SYNCHRONOUS_ECHO_POLICY_SYMMETRIC_KEY_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "services/synchronous_util/SynchronousSesameSecured.hpp"
#include "services/util/EchoOnSesame.hpp"

namespace services
{
    class SynchronousEchoPolicySymmetricKey
        : private EchoInitializationObserver
        , private EchoPolicy
        , private sesame_security::SymmetricKeyEstablishment
        , private sesame_security::SymmetricKeyEstablishmentProxy
    {
    public:
        SynchronousEchoPolicySymmetricKey(Echo& echo, EchoInitialization& echoInitialization, SynchronousSesameSecured& secured, hal::SynchronousRandomDataGenerator& randomDataGenerator);

    private:
        // Implementation of EchoInitializationObserver
        void Reset() override;
        void Initialized() override;

        // Implementation of EchoPolicy
        void RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest) override;
        void GrantingSend(ServiceProxy& proxy) override;

    private:
        // Implementation of SymmetricKeyEstablishment
        void ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv) override;

        void ReQueueWaitingProxies();

    private:
        SynchronousSesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        infra::Function<void(ServiceProxy& proxy)> onRequest;

        bool initializingSending = true;
        infra::IntrusiveList<ServiceProxy> waitingProxies;
        std::optional<std::pair<SynchronousSesameSecured::KeyType, SynchronousSesameSecured::IvType>> nextKeyPair;
    };
}

#endif
