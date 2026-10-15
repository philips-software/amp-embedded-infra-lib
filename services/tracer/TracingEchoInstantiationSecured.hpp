#ifndef SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameInstantiation.hpp"
#include "services/util/SesameInstantiationSecured.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/TracingEchoOnSesame.hpp"
#include <utility>
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace main_
{
    struct TracingEchoOnSesameSecured
    {
        template<class... Args>
        TracingEchoOnSesameSecured(services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, Args&&... sesameSecuredArgs)
            : sesameSecured(std::forward<Args>(sesameSecuredArgs)...)
            , echo(serializerFactory, echoErrorPolicy, tracer, sesameSecured.secured)
        {
        }

        void Reset()
        {
            echo.Reset();
        }

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SesameInstantiationSecured sesameSecured;
        services::TracingEchoOnSesame echo;
    };

    struct TracingEchoOnSesameSecuredSymmetricKey
        : TracingEchoOnSesameSecured
    {
        template<class... Args>
        TracingEchoOnSesameSecuredSymmetricKey(hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, Args&&... sesameSecuredArgs)
            : TracingEchoOnSesameSecured(serializerFactory, echoErrorPolicy, tracer, std::forward<Args>(sesameSecuredArgs)...)
            , policy(echo, echo, sesameSecured.secured, randomDataGenerator)
        {
        }

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::Storage<MessageSize, SplitBuffers>
        , private services::SesameSecured::Storage<MessageSize>
        , public TracingEchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredSymmetricKey(randomDataGenerator, serializerFactory, echoErrorPolicy, tracer, static_cast<Sesame::StorageBase&>(*this), static_cast<services::SesameSecured::StorageBase&>(*this), encryptors, serialCommunication, keyMaterial, initializer)
        {
        }

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredSymmetricKey::WithMessageSize<MessageSize, SplitBuffers>(randomDataGenerator, serializerFactory, echoErrorPolicy, tracer, static_cast<services::AesGcmEncryptors&>(*this), serialCommunication, keyMaterial, initializer)
        {}
    };
#endif

    struct TracingEchoOnSesameSecuredDiffieHellman
        : TracingEchoOnSesameSecured
    {
        template<class... Args>
        TracingEchoOnSesameSecuredDiffieHellman(const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, Args&&... sesameSecuredArgs)
            : TracingEchoOnSesameSecured(serializerFactory, echoErrorPolicy, tracer, std::forward<Args>(sesameSecuredArgs)...)
            , policy(crypto, echo, echo, sesameSecured.secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
        {
        }

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::Storage<MessageSize, SplitBuffers>
        , private services::SesameSecured::Storage<MessageSize>
        , public TracingEchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::Tracer& tracer, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman(crypto, keyMaterial, randomDataGenerator, serializerFactory, echoErrorPolicy, tracer, static_cast<Sesame::StorageBase&>(*this), static_cast<services::SesameSecured::StorageBase&>(*this), encryptors, serialCommunication, services::SesameSecured::KeyMaterial{}, initializer)
        {
        }

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , private services::CryptoMbedTls
        , public TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : services::CryptoMbedTls(keyMaterial, randomDataGenerator)
            , TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>(static_cast<const services::EchoPolicyDiffieHellman::Crypto&>(*this), keyMaterial, randomDataGenerator, serializerFactory, echoErrorPolicy, tracer, static_cast<services::AesGcmEncryptors&>(*this), serialCommunication, initializer)
        {}
    };
#endif
}

#endif
