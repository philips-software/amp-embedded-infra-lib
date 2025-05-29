#ifndef SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"

namespace main_
{
    struct EchoOnSesameSecured
    {
        EchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial);
        ~EchoOnSesameSecured();

        void Reset();

        services::SesameCobs cobs;
        services::SesameWindowed windowed{ cobs };
        services::SesameSecured::WithCryptoMbedTls secured;
        services::EchoOnSesame echo;
    };

    struct EchoOnSesameSecuredSymmetricKey
        : EchoOnSesameSecured
    {
        template<std::size_t MessageSize>
        struct WithMessageSize;

        EchoOnSesameSecuredSymmetricKey(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize>
    struct EchoOnSesameSecuredSymmetricKey::WithMessageSize
        : EchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : EchoOnSesameSecuredSymmetricKey(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator)
        {}

    private:
        static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
        infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
    };

    struct EchoOnSesameSecuredDiffieHellman
        : EchoOnSesameSecured
    {
        template<std::size_t MessageSize>
        struct WithMessageSize;

        EchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, infra::ConstByteRange dsaCertificate, infra::ConstByteRange dsaCertificatePrivateKey, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        infra::Creator<services::EcSecP256r1DiffieHellman, services::EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        services::EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<services::EcSecP256r1DsaVerifier, services::EcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize>
    struct EchoOnSesameSecuredDiffieHellman::WithMessageSize
        : EchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, infra::ConstByteRange dsaCertificate, infra::ConstByteRange dsaCertificatePrivateKey, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : EchoOnSesameSecuredDiffieHellman(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, dsaCertificatePrivateKey, dsaCertificate, rootCaCertificate, randomDataGenerator)
        {}

    private:
        static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
        infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
    };
}

#endif
