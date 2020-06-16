#ifndef SERVICES_TRACER_WITH_TIME_HPP
#define SERVICES_TRACER_WITH_TIME_HPP

#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracerWithTime
        : public Tracer
    {
    public:
        explicit TracerWithTime(infra::TextOutputStream& stream);

    protected:
        virtual void InsertHeader() override;
    };
}

#endif
