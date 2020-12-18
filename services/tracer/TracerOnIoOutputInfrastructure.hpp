#ifndef SERVICE_TRACER_TRACER_ON_IO_OUTPUT_INFRASTRUCTURE_HPP
#define SERVICE_TRACER_TRACER_ON_IO_OUTPUT_INFRASTRUCTURE_HPP

#include "infra/stream/IoOutputStream.hpp"
#include "services/tracer/TracerWithDateTime.hpp"

namespace main_
{
    struct TracerOnIoOutputInfrastructure
    {
        TracerOnIoOutputInfrastructure();

        infra::IoOutputStreamWriter writer;
        infra::TextOutputStream::WithErrorPolicy stream{ writer };
        services::TracerWithDateTime tracer{ stream };
    };
}

#endif
