#include "services/tracer/Tracer.hpp"

namespace services
{
    Tracer::Tracer(infra::TextOutputStream& stream)
        : stream(stream)
    {}

    infra::TextOutputStream Tracer::Trace()
    {
        stream << "\r\n";
        InsertHeader();

        return Continue();
    }

    infra::TextOutputStream Tracer::Continue()                                                                          //TICS !INT#006
    {
        return stream;
    }

    void Tracer::InsertHeader()
    {}
}
