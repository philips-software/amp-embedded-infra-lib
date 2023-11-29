#ifndef SERVICES_TRACER_HPP
#define SERVICES_TRACER_HPP

#include "infra/stream/OutputStream.hpp"

namespace services
{
    class Tracer
    {
    public:
        class TracingTextOutputStream;

        explicit Tracer(infra::TextOutputStream& stream);
        Tracer(const Tracer& other) = delete;
        Tracer& operator=(const Tracer& other) = delete;
        virtual ~Tracer() = default;

        virtual infra::TextOutputStream Continue();

#ifdef EMIL_ENABLE_GLOBAL_TRACING
        infra::TextOutputStream Trace();
#else
        class EmptyTracing
        {
        public:
            template<class T>
            EmptyTracing operator<<(T x)
            {
                return *this;
            }
        };

        EmptyTracing Trace();

    private:
        infra::StreamWriterDummy dummy;
        infra::TextOutputStream::WithErrorPolicy dummyStream{ dummy };
#endif

    protected:
        virtual void InsertHeader();
        virtual void StartTrace();

    private:
        infra::TextOutputStream& stream;
    };
}
#endif
