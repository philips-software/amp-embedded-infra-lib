#include "services/synchronous_util/SynchronousEchoInstantiationSecured.hpp"

namespace main_
{
    SynchronousEchoOnSesameSecured::SynchronousEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial)
        : sesame(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, keyMaterial)
        , echo(sesame.secured, serializerFactory)
    {}

    void SynchronousEchoOnSesameSecured::Reset()
    {
        echo.Reset();
    }

    void SynchronousEchoOnSesameSecured::Stop(const infra::Function<void()>& onDone)
    {
        sesame.Stop(onDone);
    }

    SynchronousEchoOnSesameSecuredSymmetricKey::SynchronousEchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : SynchronousEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial)
        , policy(echo, echo, sesame.secured, randomDataGenerator)
    {}

    SynchronousEchoOnSesameSecuredDiffieHellman::SynchronousEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
        hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : SynchronousEchoOnSesameSecured(storage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, services::SynchronousSesameSecured::KeyMaterial{})
        , signer{ keyMaterial.dsaCertificatePrivateKey, randomDataGenerator }
        , policy(services::SynchronousEchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, echo, echo, sesame.secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
    {}
}
