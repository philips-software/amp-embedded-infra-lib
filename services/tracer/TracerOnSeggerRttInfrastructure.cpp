#include "services/tracer/TracerOnSeggerRttInfrastructure.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace main_
{
    TracerOnSeggerRttInfrastructure::TracerOnSeggerRttInfrastructure()
    {
        services::SetGlobalTracerInstance(tracer);
    }
}
