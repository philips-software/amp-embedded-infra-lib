#include "services/tracer/TracingEchoInstantiationSecured.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"

namespace main_
{
    TracingEchoOnSesameSecured::TracingEchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
        infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, infra::ConstByteRange initInfo, services::SesameInitializer & initializer)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , windowed(windowedReceivedMessage, cobs, initInfo, initializer)
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

    TracingEchoOnSesameSecuredSymmetricKey::TracingEchoOnSesameSecuredSymmetricKey(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
        infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, infra::ConstByteRange initInfo, services::SesameInitializer & initializer)
        : TracingEchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, echoErrorPolicy, tracer, initInfo, initializer)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    TracingEchoOnSesameSecuredDiffieHellman::TracingEchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
        infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, infra::ConstByteRange initInfo, services::SesameInitializer & initializer)
        : TracingEchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{}, echoErrorPolicy, tracer, initInfo, initializer)
        , policy(crypto, echo, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator)
    {}
}
