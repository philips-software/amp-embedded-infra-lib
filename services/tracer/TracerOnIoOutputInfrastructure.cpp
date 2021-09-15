#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace main_
{
    TracerOnIoOutputInfrastructure::TracerOnIoOutputInfrastructure()
    {
        services::SetGlobalTracerInstance(tracer);
    }
}
