#include "services/tracer/TracerWithPrefix.hpp"

namespace services
{
    TracerWithPrefix::TracerWithPrefix(infra::BoundedConstString prefix, Tracer& delegate)
        : TracerToDelegate(delegate)
        , prefix(prefix)
    {}

    void TracerWithPrefix::InsertHeader()
    {
        Continue() << prefix;

        TracerToDelegate::InsertHeader();
    }
}
