#ifndef SERVICES_TRACING_RESET_HPP
#define SERVICES_TRACING_RESET_HPP

#include "hal/interfaces/Reset.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingReset
        : public hal::Reset
    {
    public:
        TracingReset(hal::Reset& reset, services::Tracer& tracer);

        virtual void ResetModule(const char* resetReason) override;

    private:
        hal::Reset& reset;
        services::Tracer& tracer;
    };
}

#endif
