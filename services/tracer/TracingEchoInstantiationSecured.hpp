#ifndef SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    struct TracingEchoOnSesameSecured
    {
        TracingEchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer);
        ~TracingEchoOnSesameSecured();

        void Reset();

        services::SesameCobs cobs;
        services::SesameWindowed windowed{ cobs };
        services::SesameSecured::WithCryptoMbedTls secured;
        services::TracingEchoOnSesame echo;
    };

    struct TracingEchoOnSesameSecuredSymmetricKey
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize>
        struct WithMessageSize;

        TracingEchoOnSesameSecuredSymmetricKey(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer);

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize>
    struct TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : TracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : TracingEchoOnSesameSecuredSymmetricKey(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator, tracer)
        {}

    private:
        static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
        infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
    };

    struct TracingEchoOnSesameSecuredDiffieHellman
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize>
        struct WithMessageSize;

        TracingEchoOnSesameSecuredDiffieHellman(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer);

        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : TracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : TracingEchoOnSesameSecuredDiffieHellman(cobsSendStorage, cobsReceivedMessage, securedSendBuffer, securedReceiveBuffer, serialCommunication, serializerFactory, crypto, dsaCertificate, rootCaCertificate, randomDataGenerator, tracer)
        {}

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif

    private:
        static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
        infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedSendBuffer;
        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameSecured::encodedMessageSize<MessageSize>> securedReceiveBuffer;
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize>::WithCryptoMbedTls
        : public TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize>(serialCommunication, serializerFactory, services::EchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, dsaCertificate, rootCaCertificate, randomDataGenerator, tracer)
        {}

        infra::Creator<services::EcSecP256r1DiffieHellman, services::EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        services::EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<services::EcSecP256r1DsaVerifier, services::EcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
