#ifndef SERVICES_TRACER_WITH_TIME_HPP
#define SERVICES_TRACER_WITH_TIME_HPP

#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracerWithTime
        : public TracerToStream
    {
    public:
        explicit TracerWithTime(infra::TextOutputStream& stream);

    protected:
        void InsertHeader() override;
    };
}

#endif
