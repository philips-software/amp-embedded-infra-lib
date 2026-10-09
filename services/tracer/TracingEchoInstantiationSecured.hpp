#ifndef SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoInstantiationSecured.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameCryptoMbedTls.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    struct TracingEchoOnSesameSecured
        : public services::Stoppable

    {
        struct SesameArgs
        {
            Sesame::CobsStorageBase& storage;
            hal::BufferedSerialCommunication& serialCommunication;
            services::AesGcmEncryptors& encryptors;
            infra::BoundedVector<uint8_t>& securedSendBuffer;
            infra::BoundedVector<uint8_t>& securedReceiveBuffer;
            const services::SesameSecured::KeyMaterial& keyMaterial = {};
            services::SesameInitializer& initializer = services::immediatelyGranted;
        };

        struct EchoArgs
        {
            services::MethodSerializerFactory& serializerFactory;
            services::Tracer& tracer;
            const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError;
        };

        TracingEchoOnSesameSecured(const EchoArgs& echoArgs, const SesameArgs& sesameArgs);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::SesameWindowed windowed;
        services::SesameSecured secured;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
    };

    struct TracingEchoOnSesameSecuredSymmetricKey
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        TracingEchoOnSesameSecuredSymmetricKey(const EchoArgs& echoArgs, const SesameArgs& sesameArgs, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(const EchoArgs& echoArgs, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredSymmetricKey({ static_cast<Sesame::CobsStorageBase&>(*this),
                                                         serialCommunication,
                                                         encryptors,
                                                         this->securedSendBuffer,
                                                         this->securedReceiveBuffer,
                                                         keyMaterial,
                                                         initializer },
                  echoArgs, randomDataGenerator)
        {}
    };

    struct TracingEchoOnSesameSecuredDiffieHellman
        : TracingEchoOnSesameSecured
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        TracingEchoOnSesameSecuredDiffieHellman(const EchoArgs& echoArgs, const SesameArgs& sesameArgs, const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , private EchoOnSesameSecured::SecuredStorage<MessageSize>
        , TracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(const EchoArgs& echoArgs, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman(echoArgs, { static_cast<Sesame::CobsStorageBase&>(*this), serialCommunication, encryptors, this->securedSendBuffer, this->securedReceiveBuffer, services::SesameSecured::KeyMaterial{}, initializer }, crypto, keyMaterial, randomDataGenerator)
        {}

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>({ serializerFactory, tracer, echoErrorPolicy }, *this, serialCommunication, services::EchoPolicyDiffieHellman::Crypto{ keyExchange, signer, verifier, keyExpander }, keyMaterial, randomDataGenerator, initializer)
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
