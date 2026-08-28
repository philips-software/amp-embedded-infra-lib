#ifndef SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/tracer/TracingSesameWindowed.hpp"
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
        TracingEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::TracingSesameWindowed windowed;
        services::SesameSecured::WithCryptoMbedTls secured;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
    };

    struct TracingEchoOnSesameSecuredSymmetricKey
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        TracingEchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredSymmetricKey(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator, tracer, echoErrorPolicy, initializer)
        {}
    };

    struct TracingEchoOnSesameSecuredDiffieHellman
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        TracingEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory,
            const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory,
            const services::EchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, crypto, dsaCertificate, rootCaCertificate, randomDataGenerator, tracer, echoErrorPolicy, initializer)
        {}

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : public TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>(serialCommunication, serializerFactory, services::EchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator, tracer, echoErrorPolicy, initializer)
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
