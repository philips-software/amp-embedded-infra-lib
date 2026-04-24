#include "services/tracer/Tracer.hpp"

namespace services
{
#if defined(EMIL_ENABLE_TRACING)
    infra::TextOutputStream Tracer::Trace()
    {
        StartTrace();
        InsertHeader();
        return Continue();
    }
#elif defined(EMIL_DISABLE_TRACING)
    Tracer::EmptyTracing Tracer::Trace()
    {
        return EmptyTracing{};
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

#if defined(EMIL_ENABLE_TRACING)
    infra::TextOutputStream TracerToStream::Continue()
    {
        return stream;
    }
#endif

    TracerToDelegate::TracerToDelegate(Tracer& delegate)
        : delegate(delegate)
    {}

#if defined(EMIL_ENABLE_TRACING)
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
