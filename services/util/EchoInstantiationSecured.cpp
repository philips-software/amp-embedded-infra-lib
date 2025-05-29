#include "services/util/EchoInstantiationSecured.hpp"

namespace main_
{
    EchoOnSesameSecured::EchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , secured(securedSendBuffer, securedReceiveBuffer, windowed, keyMaterial)
        , echo(secured, serializerFactory)
    {}

    EchoOnSesameSecured::~EchoOnSesameSecured()
    {
        cobs.Stop();
        windowed.Stop();
    }

    void EchoOnSesameSecured::Reset()
    {
        echo.Reset();
    }

    EchoOnSesameSecuredSymmetricKey::EchoOnSesameSecuredSymmetricKey(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial)
        , policy(echo, echo, secured, randomDataGenerator)
    {}

    EchoOnSesameSecuredDiffieHellman::EchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{})
        , policy(crypto, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator)
    {}
}
