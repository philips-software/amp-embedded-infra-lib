#ifndef SERVICES_TRACING_ECHO_INSTANTIATIONS
#define SERVICES_TRACING_ECHO_INSTANTIATIONS

#include "services/tracer/Tracer.hpp"
#include "services/util/EchoInstantiation.hpp"
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
}

#endif
