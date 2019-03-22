#include "infra/stream/IoOutputStream.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    namespace
    {
        Tracer* globalTracerInstance = nullptr;

#ifdef CCOLA_HOST_BUILD
        infra::IoOutputStream ioOutputStream;
        Tracer tracerDummy(ioOutputStream);
#endif
    }

    void SetGlobalTracerInstance(Tracer& tracer)
    {
        assert(globalTracerInstance == nullptr);
        globalTracerInstance = &tracer;

    }

    Tracer& GlobalTracer()
    {
#ifdef CCOLA_HOST_BUILD
        if (globalTracerInstance == nullptr)
            return tracerDummy;
#endif

        assert(globalTracerInstance != nullptr);
        return *globalTracerInstance;
    }
}
