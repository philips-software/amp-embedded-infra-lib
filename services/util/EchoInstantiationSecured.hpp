#ifndef SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP

#include "services/util/EchoInstantiation.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameInstantiationSecured.hpp"

namespace main_
{
    struct EchoOnSesameSecured
        : public services::Stoppable
    {
        EchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        SesameSecured sesame;
        services::EchoOnSesame echo;

        template<std::size_t MessageSize>
        struct SecuredStorage
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
        };
    };

    struct EchoOnSesameSecuredSymmetricKey
        : EchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        EchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , EchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : EchoOnSesameSecuredSymmetricKey(this->cobsSendStorage, this->cobsReceivedMessage, this->windowedReceivedMessage, this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator)
        {}
    };

    struct EchoOnSesameSecuredDiffieHellman
        : EchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        EchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        infra::Creator<services::EcSecP256r1DiffieHellman, services::EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        services::EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<services::EcSecP256r1DsaVerifier, services::EcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , EchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : EchoOnSesameSecuredDiffieHellman(this->cobsSendStorage, this->cobsReceivedMessage, this->windowedReceivedMessage, this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator)
        {}
    };
}

#endif
