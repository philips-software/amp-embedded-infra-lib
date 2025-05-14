#ifdef EMIL_HOST_BUILD
#include "infra/stream/IoOutputStream.hpp"
#endif
#include "services/tracer/Tracer.hpp"

namespace services
{
    namespace
    {
        Tracer* globalTracerInstance = nullptr;

#ifdef EMIL_HOST_BUILD
        infra::IoOutputStream ioOutputStream;
        TracerToStream tracerDummy(ioOutputStream);
#endif
    }

    void SetGlobalTracerInstance(Tracer& tracer)
    {
        assert(globalTracerInstance == nullptr);
        globalTracerInstance = &tracer;
    }

    Tracer& GlobalTracer()
    {
#ifdef EMIL_HOST_BUILD
        if (globalTracerInstance == nullptr)
            return tracerDummy;
#endif

        assert(globalTracerInstance != nullptr);
        return *globalTracerInstance;
    }
}
