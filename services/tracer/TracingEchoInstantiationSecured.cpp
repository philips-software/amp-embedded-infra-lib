#include "services/tracer/TracingEchoInstantiationSecured.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"

namespace main_
{
    TracingEchoOnSesameSecured::TracingEchoOnSesameSecured(EchoArgs echoArgs, SesameArgs sesameArgs)
        : cobs(sesameArgs.storage.cobsSendStorage, sesameArgs.storage.cobsReceivedMessage, sesameArgs.serialCommunication)
        , windowed(sesameArgs.storage.windowedReceivedMessage, sesameArgs.storage.windowedReceiveBuffers, cobs, sesameArgs.initializer)
        , secured(sesameArgs.encryptors, sesameArgs.securedSendBuffer, sesameArgs.securedReceiveBuffer, windowed, sesameArgs.keyMaterial)
        , echo(echoArgs.serializerFactory, echoArgs.echoErrorPolicy, echoArgs.tracer, secured)
    {}

    void TracingEchoOnSesameSecured::Reset()
    {
        echo.Reset();
    }

    void TracingEchoOnSesameSecured::Stop(const infra::Function<void()>& onDone)
    {
        this->onStopDone = onDone;
        cobs.Stop([this]()
            {
                windowed.ResetReading();
                this->onStopDone();
            });
    }

    TracingEchoOnSesameSecuredSymmetricKey::TracingEchoOnSesameSecuredSymmetricKey(EchoArgs echoArgs, SesameArgs sesameArgs, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : TracingEchoOnSesameSecured(echoArgs, sesameArgs)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    TracingEchoOnSesameSecuredDiffieHellman::TracingEchoOnSesameSecuredDiffieHellman(EchoArgs echoArgs, SesameArgs sesameArgs, const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : TracingEchoOnSesameSecured(echoArgs, sesameArgs)
        , policy(crypto, echo, echo, secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
    {}
}
