#include "services/tracer/TracingReset.hpp"

namespace services
{
    TracingReset::TracingReset(hal::Reset& reset, services::Tracer& tracer)
        : reset(reset)
        , tracer(tracer)
    {}

    void TracingReset::ResetModule(const char* resetReason)
    {
        tracer.Trace() << "Reset module: " << resetReason;
        tracer.Trace() << "...";
        tracer.Trace() << "...";

        reset.ResetModule(resetReason);
    }
}
