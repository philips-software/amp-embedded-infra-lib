#include "services/tracer/Tracer.hpp"

namespace services
{
    Tracer::Tracer(infra::TextOutputStream& stream)
        : stream(stream)
    {}

#ifdef EMIL_ENABLE_GLOBAL_TRACING
    infra::TextOutputStream Tracer::Trace()
    {
        StartTrace();
        InsertHeader();
        return Continue();
    }

    infra::TextOutputStream Tracer::Continue()
    {
        return stream;
    }
#else
    Tracer::EmptyTracing Tracer::Trace()
    {
        return EmptyTracing{};
    }

    infra::TextOutputStream Tracer::Continue()
    {
        return dummyStream;
    }
#endif

    void Tracer::InsertHeader()
    {}

    void Tracer::StartTrace()
    {
        Continue() << "\r\n";
    }
}
