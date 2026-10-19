#ifndef SERVICES_SYNCHRONOUS_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_SYNCHRONOUS_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/synchronous_util/SynchronousEchoInstantiationSecured.hpp"
#include "services/synchronous_util/SynchronousEchoPolicyDiffieHellman.hpp"
#include "services/synchronous_util/SynchronousEchoPolicySymmetricKey.hpp"
#include "services/synchronous_util/SynchronousSesameCryptoMbedTls.hpp"
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
        struct SesameArgs
        {
            Sesame::CobsStorageBase& storage;
            hal::BufferedSerialCommunication& serialCommunication;
            services::SynchronousAesGcmEncryptors& encryptors;
            infra::BoundedVector<uint8_t>& securedSendBuffer;
            infra::BoundedVector<uint8_t>& securedReceiveBuffer;
            const services::SynchronousSesameSecured::KeyMaterial& keyMaterial = {};
            services::SesameInitializer& initializer = services::immediatelyGranted;
        };

        struct EchoArgs
        {
            services::MethodSerializerFactory& serializerFactory;
            services::Tracer& tracer;
            const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError;
        };

        SynchronousTracingEchoOnSesameSecured(const EchoArgs& echoArgs, const SesameArgs& sesameArgs);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::SesameWindowed windowed;
        services::SynchronousSesameSecured secured;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
    };

    struct SynchronousTracingEchoOnSesameSecuredSymmetricKey
        : SynchronousTracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousTracingEchoOnSesameSecuredSymmetricKey(const EchoArgs& echoArgs, const SesameArgs& sesameArgs, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::SynchronousEchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousTracingEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private SynchronousEchoOnSesameSecured::SecuredStorage<MessageSize>
        , SynchronousTracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(const EchoArgs& echoArgs, services::SynchronousAesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SynchronousSesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SynchronousTracingEchoOnSesameSecuredSymmetricKey(echoArgs, { static_cast<Sesame::CobsStorageBase&>(*this), serialCommunication, encryptors, this->securedSendBuffer, this->securedReceiveBuffer, keyMaterial, initializer }, randomDataGenerator)
        {}
    };

    struct SynchronousTracingEchoOnSesameSecuredDiffieHellman
        : SynchronousTracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SynchronousTracingEchoOnSesameSecuredDiffieHellman(const EchoArgs& echoArgs, const SesameArgs& sesameArgs, const services::SynchronousEchoPolicyDiffieHellman::Crypto& crypto, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::SynchronousEchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private SynchronousEchoOnSesameSecured::SecuredStorage<MessageSize>
        , SynchronousTracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(const EchoArgs& echoArgs, services::SynchronousAesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SynchronousEchoPolicyDiffieHellman::Crypto& crypto, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SynchronousTracingEchoOnSesameSecuredDiffieHellman(echoArgs, { static_cast<Sesame::CobsStorageBase&>(*this), serialCommunication, encryptors, this->securedSendBuffer, this->securedReceiveBuffer, services::SynchronousSesameSecured::KeyMaterial{}, initializer }, crypto, keyMaterial, randomDataGenerator)
        {}

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SynchronousSesameSecuredMbedTlsEncryptors
        , public SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SynchronousEchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>({ serializerFactory, tracer, echoErrorPolicy }, *this, serialCommunication, services::SynchronousEchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, keyMaterial, randomDataGenerator, initializer)
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
