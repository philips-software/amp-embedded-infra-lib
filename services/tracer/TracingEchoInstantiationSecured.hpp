#ifndef SERVICES_TRACING_ECHO_INSTANTIATION_SECURED_HPP
#define SERVICES_TRACING_ECHO_INSTANTIATION_SECURED_HPP

#include "services/tracer/Tracer.hpp"
#include "services/util/EchoInstantiationSecured.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    template<std::size_t MessageSize>
    struct TracingEchoOnSesameSecured
    {
        TracingEchoOnSesameSecured(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : cobs(serialCommunication)
            , secured(windowed, keyMaterial)
            , echo(serializerFactory, services::echoErrorPolicyAbortOnMessageFormatError, tracer)
            , policy(echo, secured, randomDataGenerator)
        {}

        ~TracingEchoOnSesameSecured()
        {
            cobs.Stop();
            windowed.Stop();
        }

        operator services::Echo&()
        {
            return echo;
        }

        void Reset()
        {
            echo.Reset();
        }

        services::SesameCobs::WithMaxMessageSize<MessageSize> cobs;
        services::SesameWindowed windowed{ cobs };
        services::SesameSecured::WithCryptoMbedTls::WithBuffers<MessageSize> secured;
        services::TracingEchoOnSesame echo;
        services::EchoPolicySymmetricKey policy;
    };
}

#endif
