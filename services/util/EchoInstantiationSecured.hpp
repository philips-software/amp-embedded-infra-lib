#ifndef SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP

#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameInstantiation.hpp"
#include "services/util/SesameInstantiationSecured.hpp"
#include <utility>
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace main_
{
    struct EchoOnSesameSecured
    {
        template<class... Args>
        EchoOnSesameSecured(services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, Args&&... sesameSecuredArgs)
            : sesameSecured(std::forward<Args>(sesameSecuredArgs)...)
            , echo(sesameSecured.secured, serializerFactory, echoErrorPolicy)
        {
        }

        void Reset()
        {
            echo.Reset();
        }

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        SesameInstantiationSecured sesameSecured;
        services::EchoOnSesame echo;
    };

    struct EchoOnSesameSecuredSymmetricKey
        : EchoOnSesameSecured
    {
        template<class... Args>
        EchoOnSesameSecuredSymmetricKey(hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, Args&&... sesameSecuredArgs)
            : EchoOnSesameSecured(serializerFactory, echoErrorPolicy, std::forward<Args>(sesameSecuredArgs)...)
            , policy(echo, echo, sesameSecured.secured, randomDataGenerator)
        {
        }

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        services::EchoPolicySymmetricKey policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredSymmetricKey::WithMessageSize
        : private Sesame::Storage<MessageSize, SplitBuffers>
        , private services::SesameSecured::Storage<MessageSize>
        , public EchoOnSesameSecuredSymmetricKey
    {
        WithMessageSize(hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, const services::SesameSecured::KeyMaterial& keyMaterial = {}, services::SesameInitializer& initializer = services::immediatelyGranted)
            : EchoOnSesameSecuredSymmetricKey(randomDataGenerator, serializerFactory, echoErrorPolicy, static_cast<Sesame::StorageBase&>(*this), static_cast<services::SesameSecured::StorageBase&>(*this), encryptors, serialCommunication, keyMaterial, initializer)
        {
        }

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredSymmetricKey::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public EchoOnSesameSecuredSymmetricKey::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : EchoOnSesameSecuredSymmetricKey::WithMessageSize<MessageSize, SplitBuffers>(randomDataGenerator, serializerFactory, echoErrorPolicy, static_cast<services::AesGcmEncryptors&>(*this), serialCommunication, keyMaterial, initializer)
        {}
    };
#endif

    struct EchoOnSesameSecuredDiffieHellman
        : EchoOnSesameSecured
    {
        template<class... Args>
        EchoOnSesameSecuredDiffieHellman(const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, Args&&... sesameSecuredArgs)
            : EchoOnSesameSecured(serializerFactory, echoErrorPolicy, std::forward<Args>(sesameSecuredArgs)...)
            , policy(crypto, echo, echo, sesameSecured.secured, keyMaterial.dsaCertificate, keyMaterial.rootCaCertificate, randomDataGenerator)
        {
        }

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        services::EchoPolicyDiffieHellman policy;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredDiffieHellman::WithMessageSize
        : private Sesame::Storage<MessageSize, SplitBuffers>
        , private services::SesameSecured::Storage<MessageSize>
        , public EchoOnSesameSecuredDiffieHellman
    {
        WithMessageSize(const services::EchoPolicyDiffieHellman::Crypto& crypto, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& echoErrorPolicy, services::AesGcmEncryptors& encryptors, hal::BufferedSerialCommunication& serialCommunication, services::SesameInitializer& initializer = services::immediatelyGranted)
            : EchoOnSesameSecuredDiffieHellman(crypto, keyMaterial, randomDataGenerator, serializerFactory, echoErrorPolicy, static_cast<Sesame::StorageBase&>(*this), static_cast<services::SesameSecured::StorageBase&>(*this), encryptors, serialCommunication, services::SesameSecured::KeyMaterial{}, initializer)
        {
        }

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , private services::CryptoMbedTls
        , public EchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : services::CryptoMbedTls(keyMaterial, randomDataGenerator)
            , EchoOnSesameSecuredDiffieHellman::WithMessageSize<MessageSize, SplitBuffers>(static_cast<const services::EchoPolicyDiffieHellman::Crypto&>(*this), keyMaterial, randomDataGenerator, serializerFactory, echoErrorPolicy, static_cast<services::AesGcmEncryptors&>(*this), serialCommunication, initializer)
        {}
    };
#endif
}

#endif
