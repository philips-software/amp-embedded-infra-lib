#ifndef SERVICES_TRACING_ECHO_INSTANTIATIONS
#define SERVICES_TRACING_ECHO_INSTANTIATIONS

#include "services/tracer/Tracer.hpp"
#include "services/tracer/TracingSesameWindowed.hpp"
#include "services/util/EchoInstantiation.hpp"

namespace main_
{
    template<std::size_t MessageSize>
    struct TracingEchoOnSesame
    {
        TracingEchoOnSesame(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer)
            : cobs(serialCommunication)
            , windowed(cobs, tracer)
            , echo(windowed, serializerFactory)
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

        services::SesameCobs::WithMaxMessageSize<MessageSize> cobs;
        services::TracingSesameWindowed windowed;
        services::EchoOnSesame echo;
    };
}

#endif
