#ifndef SERVICES_ECHO_POLICY_SYMMETRIC_KEY_HPP
#define SERVICES_ECHO_POLICY_SYMMETRIC_KEY_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/SesameSecured.hpp"

namespace services
{
    class EchoPolicySymmetricKey
        : private EchoInitializationObserver
        , private EchoPolicy
        , private sesame_security::SymmetricKeyEstablishment
        , private sesame_security::SymmetricKeyEstablishmentProxy
    {
    public:
        EchoPolicySymmetricKey(EchoWithPolicy& echo, EchoInitialization& echoInitialization, SesameSecured& secured, hal::SynchronousRandomDataGenerator& randomDataGenerator);

    private:
        // Implementation of EchoInitializationObserver
        void Initialized() override;

        // Implementation of EchoPolicy
        void RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest) override;
        void GrantingSend(ServiceProxy& proxy) override;

    private:
        // Implementation of SymmetricKeyEstablishment
        void ActivateNewKeyMaterial(infra::ConstByteRange key, infra::ConstByteRange iv) override;

        void ReQueueWaitingProxies();

    private:
        SesameSecured& secured;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        infra::Function<void(ServiceProxy& proxy)> onRequest;

        bool initializingSending = true;
        infra::IntrusiveList<ServiceProxy> waitingProxies;
        std::optional<std::pair<std::array<uint8_t, 16>, std::array<uint8_t, 16>>> nextKeyPair;
    };
}

#endif
