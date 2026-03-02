#include "services/tracer/TracingEchoInstantiationSecured.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"

namespace main_
{
    TracingEchoOnSesameSecured::TracingEchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , windowed(windowedReceivedMessage, cobs)
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
                windowed.Stop();
                this->onStopDone();
            });
    }

    TracingEchoOnSesameSecuredSymmetricKey::TracingEchoOnSesameSecuredSymmetricKey(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer)
        : TracingEchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, echoErrorPolicy, tracer)
        , policy(echo, echo, secured, randomDataGenerator, echoErrorPolicy)
    {}

    TracingEchoOnSesameSecuredDiffieHellman::TracingEchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer)
        : TracingEchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{}, echoErrorPolicy, tracer)
        , policy(crypto, echo, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator, echoErrorPolicy)
    {}
}
