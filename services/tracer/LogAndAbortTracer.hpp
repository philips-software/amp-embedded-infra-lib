#ifndef SERVICES_UTIL_LOGANDABORTTRACER_HPP
#define SERVICES_UTIL_LOGANDABORTTRACER_HPP

#include "infra/util/LogAndAbort.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    using LogAndAbortTracerProvider = infra::Function<services::Tracer&()>;

    // This will overwrite any previously registered LogAndAbort hook
    void RegisterLogAndAbortTracerProvider(LogAndAbortTracerProvider tracerProvider);
}

#endif // SERVICES_UTIL_LOGANDABORTTRACER_HPP
