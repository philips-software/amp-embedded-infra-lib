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
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, infra::ConstByteRange dsaCertificate, infra::ConstByteRange dsaCertificatePrivateKey, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : EchoOnSesameSecured(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SesameSecured::KeyMaterial{})
        , signer{ dsaCertificatePrivateKey, randomDataGenerator }
        , policy(services::EchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, echo, secured, dsaCertificate, rootCaCertificate, randomDataGenerator)
    {}
}
