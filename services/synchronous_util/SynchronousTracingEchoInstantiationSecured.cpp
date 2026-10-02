#include "services/synchronous_util/SynchronousTracingEchoInstantiationSecured.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"

namespace main_
{
    SynchronousTracingEchoOnSesameSecured::SynchronousTracingEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : cobs(storage.cobsSendStorage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedReceivedMessage, storage.windowedReceiveBuffers, cobs, initializer)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
        , echo(serializerFactory, echoErrorPolicy, tracer, secured)
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

    SynchronousTracingEchoOnSesameSecuredSymmetricKey::SynchronousTracingEchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : SynchronousTracingEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, tracer, echoErrorPolicy, initializer)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    SynchronousTracingEchoOnSesameSecuredDiffieHellman::SynchronousTracingEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousEchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : SynchronousTracingEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SynchronousSesameSecured::KeyMaterial{}, tracer, echoErrorPolicy, initializer)
        , policy(crypto, echo, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator)
    {}
}
