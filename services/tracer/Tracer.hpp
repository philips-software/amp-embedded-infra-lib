#ifndef SERVICES_TRACER_HPP
#define SERVICES_TRACER_HPP

#include "infra/stream/OutputStream.hpp"

namespace services
{
    class Tracer
    {
    public:
        explicit Tracer(infra::TextOutputStream& stream);
        Tracer(const Tracer& other) = delete;
        Tracer& operator=(const Tracer& other) = delete;
        virtual ~Tracer() = default;

        virtual infra::TextOutputStream Trace();
        virtual infra::TextOutputStream Continue();

    protected:
        virtual void InsertHeader();

    private:
        infra::TextOutputStream& stream;
    };
}

#endif
