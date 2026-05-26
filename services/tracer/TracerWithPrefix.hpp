#ifndef SERVICES_TRACER_WITH_PREFIX_HPP
#define SERVICES_TRACER_WITH_PREFIX_HPP

#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracerWithPrefix
        : public TracerToDelegate
    {
    public:
        TracerWithPrefix(infra::BoundedConstString prefix, Tracer& delegate);

    protected:
        void InsertHeader() override;

    private:
        infra::BoundedConstString prefix;
    };
}

#endif
