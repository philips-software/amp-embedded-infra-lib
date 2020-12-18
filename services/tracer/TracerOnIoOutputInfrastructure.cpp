#include "services/tracer/GlobalTracer.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"

namespace main_
{
    TracerOnIoOutputInfrastructure::TracerOnIoOutputInfrastructure()
    {
        services::SetGlobalTracerInstance(tracer);
    }
}
