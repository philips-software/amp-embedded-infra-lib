#include "services/util/TracingEchoInstantiationSecured.hpp"

namespace main_
{
    TracingEchoOnSesameSecured::TracingEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::AesGcmEncryption& sendEncryption, services::AesGcmEncryption& receiveEncryption,
        const services::SesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : cobs(storage.cobsSendStorage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedReceivedMessage, storage.windowedReceiveBuffers, cobs, initializer)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, sendEncryption, receiveEncryption, keyMaterial)
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

    TracingEchoOnSesameSecuredDiffieHellman::TracingEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
        services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy, services::SesameInitializer& initializer)
        : TracingEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, crypto.sendEncryption, crypto.receiveEncryption, services::SesameSecured::KeyMaterial{}, tracer, echoErrorPolicy, initializer)
        , policy(crypto.keyEstablishment, echo, echo, secured, dsaCertificate, rootCaCertificate)
    {}
}
