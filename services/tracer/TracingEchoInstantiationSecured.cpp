#include "services/tracer/TracingEchoInstantiationSecured.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"

namespace main_
{
    TracingEchoOnSesameSecured::TracingEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : cobs(storage.cobsSendStorage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedReceivedMessage, storage.windowedReceiveBuffers, cobs, tracer, initializer)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
        , echo(serializerFactory, echoErrorPolicy, tracer, secured)
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

    TracingEchoOnSesameSecuredSymmetricKey::TracingEchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : TracingEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, tracer, echoErrorPolicy, initializer)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    TracingEchoOnSesameSecuredDiffieHellman::TracingEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : TracingEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{}, tracer, echoErrorPolicy, initializer)
        , policy(crypto, echo, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator)
    {}
}
