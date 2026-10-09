#ifndef SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_UTIL_ECHO_INSTANTIATION_SECURED_HPP

#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameInstantiationSecured.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace main_
{
    template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
    struct EchoOnSesameSecuredSymmetricKey
        : public SesameInstantiationSecured<MessageSize, SplitBuffers>
    {
        using SesameInstantiationSecured<MessageSize, SplitBuffers>::SesameInstantiationSecured;

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredSymmetricKey<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public EchoOnSesameSecuredSymmetricKey<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : EchoOnSesameSecuredSymmetricKey<MessageSize, SplitBuffers>(*this, serialCommunication, keyMaterial, initializer)
            , echo(this->secured, serializerFactory, echoErrorPolicy)
            , policy(echo, echo, this->secured, randomDataGenerator)
        {}

        void Reset()
        {
            echo.Reset();
        }

        services::EchoOnSesame echo;
        services::EchoPolicySymmetricKey policy;
    };
#endif

    template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
    struct EchoOnSesameSecuredDiffieHellman
        : public SesameInstantiationSecured<MessageSize, SplitBuffers>
    {
        using SesameInstantiationSecured<MessageSize, SplitBuffers>::SesameInstantiationSecured;

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct EchoOnSesameSecuredDiffieHellman<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public EchoOnSesameSecuredDiffieHellman<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : EchoOnSesameSecuredDiffieHellman<MessageSize, SplitBuffers>(*this, serialCommunication, services::SesameSecured::KeyMaterial{}, initializer)
            , echo(this->secured, serializerFactory, echoErrorPolicy)
            , policy(echo, echo, this->secured, keyMaterial, randomDataGenerator)
        {}

        void Reset()
        {
            echo.Reset();
        }

        services::EchoOnSesame echo;
        services::EchoPolicyDiffieHellman::WithCryptoMbedTls policy;
    };
#endif
}

#endif
