#ifndef SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACER_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/SesameInstantiationSecured.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/TracingEchoOnSesame.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace main_
{
    template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
    struct TracingEchoOnSesameSecuredSymmetricKey
        : public SesameInstantiationSecured<MessageSize, SplitBuffers>
    {
        using SesameInstantiationSecured<MessageSize, SplitBuffers>::SesameInstantiationSecured;

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredSymmetricKey<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public TracingEchoOnSesameSecuredSymmetricKey<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredSymmetricKey<MessageSize, SplitBuffers>(*this, serialCommunication, keyMaterial, initializer)
            , echo(serializerFactory, echoErrorPolicy, tracer, this->secured)
            , policy(echo, echo, this->secured, randomDataGenerator)
        {}

        void Reset()
        {
            echo.Reset();
        }

        services::TracingEchoOnSesame echo;
        services::EchoPolicySymmetricKey policy;
    };
#endif

    template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
    struct TracingEchoOnSesameSecuredDiffieHellman
        : public SesameInstantiationSecured<MessageSize, SplitBuffers>
    {
        using SesameInstantiationSecured<MessageSize, SplitBuffers>::SesameInstantiationSecured;

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif
    };

#ifdef EMIL_USE_MBEDTLS
    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesameSecuredDiffieHellman<MessageSize, SplitBuffers>::WithCryptoMbedTls
        : private services::SesameSecuredMbedTlsEncryptors
        , public TracingEchoOnSesameSecuredDiffieHellman<MessageSize, SplitBuffers>
    {
        WithCryptoMbedTls(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::EchoPolicyDiffieHellman::KeyMaterial& keyMaterial,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer, const services::EchoErrorPolicy& echoErrorPolicy = services::echoErrorPolicyAbortOnMessageFormatError, services::SesameInitializer& initializer = services::immediatelyGranted)
            : TracingEchoOnSesameSecuredDiffieHellman<MessageSize, SplitBuffers>(*this, serialCommunication, services::SesameSecured::KeyMaterial{}, initializer)
            , echo(serializerFactory, echoErrorPolicy, tracer, this->secured)
            , policy(echo, echo, this->secured, keyMaterial, randomDataGenerator)
        {}

        void Reset()
        {
            echo.Reset();
        }

        services::TracingEchoOnSesame echo;
        services::EchoPolicyDiffieHellman::WithCryptoMbedTls policy;
    };
#endif
}

#endif
