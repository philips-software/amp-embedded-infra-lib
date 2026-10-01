#ifndef SERVICES_TRACING_OUTPUT_STREAM_HPP
#define SERVICES_TRACING_OUTPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingStreamWriter
        : public infra::StreamWriter
    {
    public:
        TracingStreamWriter(infra::StreamWriter& writer, services::Tracer& tracer);

        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;

        std::size_t ConstructSaveMarker() const override;
        std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        infra::ByteRange SaveState(std::size_t marker) override;
        void RestoreState(infra::ByteRange range) override;
        infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        infra::StreamWriter& writer;
        services::Tracer& tracer;
    };

    class TracingAsciiStreamWriter
        : public TracingStreamWriter
    {
    public:
        TracingAsciiStreamWriter(infra::StreamWriter& writer, services::Tracer& tracer);

        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;

    private:
        infra::StreamWriter& writer;
        services::Tracer& tracer;
    };

    template<class Writer>
    class TracingOutputStreamBase
        : public infra::DataOutputStream::WithWriter<Writer>
    {
    public:
        TracingOutputStreamBase(infra::DataOutputStream& stream, services::Tracer& tracer)
            : infra::DataOutputStream::WithWriter<Writer>(stream.Writer(), tracer)
        {}

        using infra::DataOutputStream::WithWriter<Writer>::WithWriter;
    };

    using TracingOutputStream = TracingOutputStreamBase<TracingStreamWriter>;
    using TracingAsciiOutputStream = TracingOutputStreamBase<TracingAsciiStreamWriter>;
}

#endif
