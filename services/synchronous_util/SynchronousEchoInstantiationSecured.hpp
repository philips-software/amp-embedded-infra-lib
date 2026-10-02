#ifndef SERVICES_SYNCHRONOUS_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_SYNCHRONOUS_ECHO_INSTANTIATION_SECURED_HPP

#include "services/synchronous_util/SynchronousEchoPolicyDiffieHellman.hpp"
#include "services/synchronous_util/SynchronousEchoPolicySymmetricKey.hpp"
#include "services/synchronous_util/SynchronousSesameInstantiationSecured.hpp"
#include "services/util/EchoInstantiation.hpp"

namespace main_
{
    struct SynchronousEchoOnSesameSecured
        : public services::Stoppable
    {
        SynchronousEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        SynchronousSesameSecured sesame;
        services::EchoOnSesame echo;

        template<std::size_t MessageSize>
        struct SecuredStorage
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SynchronousSesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SynchronousSesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
        };
    };

    struct SynchronousEchoOnSesameSecuredSymmetricKey
        : SynchronousEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousEchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::SynchronousEchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private SynchronousEchoOnSesameSecured::SecuredStorage<MessageSize>
        , SynchronousEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : SynchronousEchoOnSesameSecuredSymmetricKey(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator)
        {}
    };

    struct SynchronousEchoOnSesameSecuredDiffieHellman
        : SynchronousEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        infra::Creator<services::SynchronousEcSecP256r1DiffieHellman, services::SynchronousEcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        services::SynchronousEcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<services::SynchronousEcSecP256r1DsaVerifier, services::SynchronousEcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
        services::SynchronousEchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private SynchronousEchoOnSesameSecured::SecuredStorage<MessageSize>
        , SynchronousEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : SynchronousEchoOnSesameSecuredDiffieHellman(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator)
        {}
    };
}

#endif
