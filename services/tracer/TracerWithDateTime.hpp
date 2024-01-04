#ifndef SERVICES_TRACER_WITH_DATE_TIME_HPP
#define SERVICES_TRACER_WITH_DATE_TIME_HPP

#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracerWithDateTime
        : public TracerToStream
    {
    public:
        explicit TracerWithDateTime(infra::TextOutputStream& stream);

    protected:
        void InsertHeader() override;
    };
}

#endif
