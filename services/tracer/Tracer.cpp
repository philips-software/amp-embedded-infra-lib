#include "services/tracer/Tracer.hpp"

namespace services
{
#if defined(EMBEDDED_INFRA_SERVICES_TRACER_ENABLED)
    infra::TextOutputStream Tracer::Trace()
    {
        StartTrace();
        InsertHeader();
        return Continue();
    }
#elif defined(EMBEDDED_INFRA_SERVICES_TRACER_DISABLED)
    Tracer::EmptyTracing Tracer::Trace()
    {
        return emptyTracing;
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

    TracerToStream::TracerToStream(infra::TextOutputStream& stream)
        : stream(stream)
    {}

#if defined(EMBEDDED_INFRA_SERVICES_TRACER_ENABLED)
    infra::TextOutputStream TracerToStream::Continue()
    {
        return stream;
    }
#endif

    TracerToDelegate::TracerToDelegate(Tracer& delegate)
        : delegate(delegate)
    {}

#if defined(EMBEDDED_INFRA_SERVICES_TRACER_ENABLED)
    infra::TextOutputStream TracerToDelegate::Continue()
    {
        return delegate.Continue();
    }
#endif

    void TracerToDelegate::InsertHeader()
    {
        delegate.InsertHeader();
    }

    void TracerToDelegate::StartTrace()
    {
        delegate.StartTrace();
    }
}
