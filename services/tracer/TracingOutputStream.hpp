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

    class TracingOutputStream
        : public infra::DataOutputStream::WithWriter<TracingStreamWriter>
    {
    public:
        TracingOutputStream(infra::DataOutputStream& stream, services::Tracer& tracer);

        using DataOutputStream::WithWriter<TracingStreamWriter>::WithWriter;
    };
}

#endif
