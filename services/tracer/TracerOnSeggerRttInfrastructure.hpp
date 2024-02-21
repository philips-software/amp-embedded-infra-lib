#ifndef SERVICE_TRACER_TRACER_ON_SEGGER_RTT_INFRASTRUCTURE_HPP
#define SERVICE_TRACER_TRACER_ON_SEGGER_RTT_INFRASTRUCTURE_HPP

#include "services/tracer/StreamWriterOnSeggerRtt.hpp"
#include "services/tracer/TracerWithDateTime.hpp"

namespace main_
{
    struct TracerOnSeggerRttInfrastructure
    {
        TracerOnSeggerRttInfrastructure();

        services::StreamWriterOnSeggerRtt writer;
        infra::TextOutputStream::WithErrorPolicy stream{ writer };
        services::TracerWithDateTime tracer{ stream };
    };
}

#endif
