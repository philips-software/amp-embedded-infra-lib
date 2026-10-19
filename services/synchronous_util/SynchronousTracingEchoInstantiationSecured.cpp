#include "services/synchronous_util/SynchronousTracingEchoInstantiationSecured.hpp"
#include "services/util/SesameInstantiation.hpp"

namespace main_
{
    SynchronousTracingEchoOnSesameSecured::SynchronousTracingEchoOnSesameSecured(const EchoArgs& echoArgs, const SesameArgs& sesameArgs)
        : cobs(sesameArgs.storage.cobsSendStorage, sesameArgs.storage.cobsReceivedMessage, sesameArgs.serialCommunication)
        , windowed(sesameArgs.storage.windowedReceivedMessage, sesameArgs.storage.windowedReceiveBuffers, cobs, sesameArgs.initializer)
        , secured(sesameArgs.encryptors, sesameArgs.securedSendBuffer, sesameArgs.securedReceiveBuffer, windowed, sesameArgs.keyMaterial)
        , echo(echoArgs.serializerFactory, echoArgs.echoErrorPolicy, echoArgs.tracer, secured)
    {}

    void SynchronousTracingEchoOnSesameSecured::Reset()
    {
        echo.Reset();
    }

    void SynchronousTracingEchoOnSesameSecured::Stop(const infra::Function<void()>& onDone)
    {
        this->onStopDone = onDone;
        cobs.Stop([this]()
            {
                windowed.ResetReading();
                this->onStopDone();
            });
    }

    SynchronousTracingEchoOnSesameSecuredSymmetricKey::SynchronousTracingEchoOnSesameSecuredSymmetricKey(const EchoArgs& echoArgs, const SesameArgs& sesameArgs, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : SynchronousTracingEchoOnSesameSecured(echoArgs, sesameArgs)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    SynchronousTracingEchoOnSesameSecuredDiffieHellman::SynchronousTracingEchoOnSesameSecuredDiffieHellman(const EchoArgs& echoArgs, const SesameArgs& sesameArgs, const services::SynchronousEchoPolicyDiffieHellman::Crypto& crypto, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : SynchronousTracingEchoOnSesameSecured(echoArgs, sesameArgs)
        , policy(crypto, echo, echo, secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
    {}
}
