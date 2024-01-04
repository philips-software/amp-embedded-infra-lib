#include "services/tracer/Tracer.hpp"

namespace services
{
#ifdef EMIL_ENABLE_GLOBAL_TRACING
    infra::TextOutputStream Tracer::Trace()
    {
        StartTrace();
        InsertHeader();
        return Continue();
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

    TracerToStream::TracerToStream(infra::TextOutputStream& stream)
        : stream(stream)
    {}

    infra::TextOutputStream TracerToStream::Continue()
    {
        return stream;
    }

    TracerToDelegate::TracerToDelegate(Tracer& delegate)
        : delegate(delegate)
    {}

    infra::TextOutputStream TracerToDelegate::Continue()
    {
        return delegate.Continue();
    }

    void TracerToDelegate::InsertHeader()
    {
        delegate.InsertHeader();
    }

    void TracerToDelegate::StartTrace()
    {
        delegate.StartTrace();
    }
}
