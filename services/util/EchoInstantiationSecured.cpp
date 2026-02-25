#include "services/util/EchoInstantiationSecured.hpp"

namespace main_
{
    EchoOnSesameSecured::EchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , windowed(windowedReceivedMessage, cobs)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
        , echo(secured, serializerFactory)
    {}

    void EchoOnSesameSecured::Reset()
    {
        echo.Reset();
    }

    void EchoOnSesameSecured::Stop(const infra::Function<void()>& onDone)
    {
        this->onStopDone = onDone;
        cobs.Stop([this]()
            {
                windowed.Stop();
                this->onStopDone();
            });
    }

    EchoOnSesameSecuredSymmetricKey::EchoOnSesameSecuredSymmetricKey(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    EchoOnSesameSecuredDiffieHellman::EchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{})
        , signer{ keyMaterial.dsaCertificatePrivateKey, randomDataGenerator }
        , policy(services::EchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, echo, echo, secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
    {}
}
