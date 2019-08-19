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

        virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

        virtual std::size_t ConstructSaveMarker() const override;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        virtual infra::ByteRange SaveState(std::size_t marker) override;
        virtual void RestoreState(infra::ByteRange range) override;
        virtual infra::ByteRange Overwrite(std::size_t marker) override;

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
