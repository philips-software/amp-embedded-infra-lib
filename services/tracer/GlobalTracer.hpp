#ifndef SERVICES_GLOBAL_TRACER_HPP
#define SERVICES_GLOBAL_TRACER_HPP

#include "services/tracer/Tracer.hpp"

namespace services
{
    void SetGlobalTracerInstance(Tracer& tracer);
    Tracer& GlobalTracer();
}

#endif
