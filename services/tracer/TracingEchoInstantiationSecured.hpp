#ifndef SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoInstantiationSecured.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    struct TracingEchoOnSesameSecured
        : public services::Stoppable
    {
        TracingEchoOnSesameSecured(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, services::Tracer& tracer);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::SesameWindowed windowed{ cobs };
        services::SesameSecured::WithCryptoMbedTls secured;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
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
        : private EchoOnSesame::CobsStorage<MessageSize>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : TracingEchoOnSesameSecuredSymmetricKey(this->cobsSendStorage, this->cobsReceivedMessage, this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator, tracer)
        {}
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
        : private EchoOnSesame::CobsStorage<MessageSize>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : TracingEchoOnSesameSecuredDiffieHellman(this->cobsSendStorage, this->cobsReceivedMessage, this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, crypto, dsaCertificate, rootCaCertificate, randomDataGenerator, tracer)
        {}

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize>::WithCryptoMbedTls
        : public TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize>(serialCommunication, serializerFactory, services::EchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator, tracer)
            , signer(keyMaterial.dsaCertificatePrivateKey, randomDataGenerator)
        {}

        infra::Creator<services::EcSecP256r1DiffieHellman, services::EcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        services::EcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<services::EcSecP256r1DsaVerifier, services::EcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
