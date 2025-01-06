#ifndef SERVICES_TRACING_ECHO_INSTANTIATION_HPP
#define SERVICES_TRACING_ECHO_INSTANTIATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoInstantiation.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    template<std::size_t MessageSize>
    struct TracingEchoOnSesame
    {
        TracingEchoOnSesame(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer)
            : cobs(serialCommunication)
            , echo(serializerFactory, services::echoErrorPolicyAbortOnMessageFormatError, tracer, windowed)
        {}

        ~TracingEchoOnSesame()
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
        services::TracingEchoOnSesame echo;
    };

    template<std::size_t MessageSize>
    struct TracingEchoOnSesameSecured
    {
        TracingEchoOnSesameSecured(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, const services::SesameSecured::KeyMaterial& keyMaterial, hal::SynchronousRandomDataGenerator& randomDataGenerator, services::Tracer& tracer)
            : cobs(serialCommunication)
            , secured(windowed, keyMaterial)
            , echo(serializerFactory, services::echoErrorPolicyAbortOnMessageFormatError, tracer, secured, randomDataGenerator)
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
        services::SesameSecured::WithBuffers<MessageSize> secured;
        services::TracingEchoOnStreamsDescendant<services::EchoOnSesameSymmetricKey> echo;
    };

#ifdef EMIL_HAL_GENERIC
    template<std::size_t MessageSize>
    struct TracingEchoOnUart
        : EchoOnUartBase<MessageSize>
    {
        TracingEchoOnUart(infra::BoundedConstString portName, services::Tracer& tracer)
            : EchoOnUartBase<MessageSize>(portName)
            , echoOnSesame(this->bufferedSerial, this->serializerFactory, tracer)
        {}

        main_::TracingEchoOnSesame<MessageSize> echoOnSesame;

        services::Echo& echo{ echoOnSesame.echo };
    };
#endif
}

#endif
