#include "services/tracer/Tracer.hpp"

namespace services
{
    Tracer::Tracer(infra::TextOutputStream& stream)
        : stream(stream)
    {}

#if defined(EMIL_ENABLE_TRACING)
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
#elif defined(EMIL_DISABLE_TRACING)
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
