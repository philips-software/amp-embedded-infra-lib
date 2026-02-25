#include "services/tracer/TracingEchoInstantiationSecured.hpp"

namespace main_
{
    TracingEchoOnSesameSecured::TracingEchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , windowed(windowedReceivedMessage, cobs)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
        , echo(serializerFactory, services::echoErrorPolicyAbort, tracer, secured)
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
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
        : TracingEchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, tracer)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    TracingEchoOnSesameSecuredDiffieHellman::TracingEchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
        : TracingEchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{}, tracer)
        , policy(crypto, echo, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator)
    {}
}
