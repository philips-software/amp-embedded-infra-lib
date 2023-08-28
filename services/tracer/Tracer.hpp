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

        virtual infra::TextOutputStream Trace();
        virtual infra::TextOutputStream Continue();

    public:
    protected:
        virtual void InsertHeader();

    private:
        infra::TextOutputStream& stream;
    };

#define EMIL_ENABLE_GLOBAL_TRACING

#ifdef EMIL_ENABLE_GLOBAL_TRACING
    static constexpr auto GlobalTracingEnabled = true;
#else
    static constexpr auto GlobalTracingEnabled = false;
#endif

    enum class TracingLevel : uint8_t
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    static auto GlobalTracingLevel = TracingLevel::DEBUG;

    template<class T, TracingLevel L>
    void TraceWithLevel(const T& trace)
    {
        // In order for compiler to optimize out code enclosed in callable trace,
        // this function needs to be a template and hence instantiated in using compilation units.
        if constexpr (GlobalTracingEnabled)
            if (L >= GlobalTracingLevel)
                trace();
    }

    template<class T>
    void TraceDebug(const T& trace)
    {
        TraceWithLevel<T, TracingLevel::DEBUG>(trace);
    };

    template<class T>
    void TraceInfo(const T& trace)
    {
        TraceWithLevel<T, TracingLevel::INFO>(trace);
    };

    template<class T>
    void TraceWarn(const T& trace)
    {
        TraceWithLevel<T, TracingLevel::WARN>(trace);
    };

    template<class T>
    void TraceError(const T& trace)
    {
        TraceWithLevel<T, TracingLevel::ERROR>(trace);
    };

    template<class T>
    void TraceFatal(const T& trace)
    {
        TraceWithLevel<T, TracingLevel::FATAL>(trace);
    };
}

#endif
