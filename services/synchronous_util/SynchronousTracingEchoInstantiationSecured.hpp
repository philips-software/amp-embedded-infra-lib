#ifndef SERVICES_SYNCHRONOUS_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_SYNCHRONOUS_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/synchronous_util/SynchronousEchoInstantiationSecured.hpp"
#include "services/synchronous_util/SynchronousEchoPolicyDiffieHellman.hpp"
#include "services/synchronous_util/SynchronousEchoPolicySymmetricKey.hpp"
#include "services/synchronous_util/SynchronousSesameSecured.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    struct SynchronousTracingEchoOnSesameSecured
        : public services::Stoppable
    {
        SynchronousTracingEchoOnSesameSecured(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial,
            services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::SesameWindowed windowed;
        services::SynchronousSesameSecured::WithCryptoMbedTls secured;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
    };

    struct SynchronousTracingEchoOnSesameSecuredSymmetricKey
        : SynchronousTracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousTracingEchoOnSesameSecuredSymmetricKey(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        services::SynchronousEchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousTracingEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private SynchronousEchoOnSesameSecured::SecuredStorage<MessageSize>
        , SynchronousTracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SynchronousTracingEchoOnSesameSecuredSymmetricKey(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, keyMaterial, randomDataGenerator, tracer, echoErrorPolicy, initializer)
        {}
    };

    struct SynchronousTracingEchoOnSesameSecuredDiffieHellman
        : SynchronousTracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousTracingEchoOnSesameSecuredDiffieHellman(Sesame::CobsStorageBase& storage, infra::BoundedVector<uint8_t>& securedSendBuffer, infra::BoundedVector<uint8_t>& securedReceiveBuffer,
            hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory,
            const services::SynchronousEchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted);

        services::SynchronousEchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private SynchronousEchoOnSesameSecured::SecuredStorage<MessageSize>
        , SynchronousTracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory,
            const services::SynchronousEchoPolicyDiffieHellman::Crypto& crypto, infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SynchronousTracingEchoOnSesameSecuredDiffieHellman(static_cast<Sesame::CobsStorageBase&>(*this), this->securedSendBuffer, this->securedReceiveBuffer, serialCommunication, serializerFactory, crypto, dsaCertificate, rootCaCertificate, randomDataGenerator, tracer, echoErrorPolicy, initializer)
        {}

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : public SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>(serialCommunication, serializerFactory, services::SynchronousEchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator, tracer, echoErrorPolicy, initializer)
            , signer(keyMaterial.dsaCertificatePrivateKey, randomDataGenerator)
        {}

        infra::Creator<services::SynchronousEcSecP256r1DiffieHellman, services::SynchronousEcSecP256r1DiffieHellmanMbedTls, void(hal::SynchronousRandomDataGenerator& randomDataGenerator)> keyExchange;
        services::SynchronousEcSecP256r1DsaSignerMbedTls signer;
        infra::Creator<services::SynchronousEcSecP256r1DsaVerifier, services::SynchronousEcSecP256r1DsaVerifierMbedTls, void(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)> verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
    };
#endif
}

#endif
