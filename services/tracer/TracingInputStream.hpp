#ifndef SERVICES_TRACING_INPUT_STREAM_HPP
#define SERVICES_TRACING_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingStreamReader
        : public infra::StreamReader
    {
    public:
        TracingStreamReader(infra::StreamReader& reader, services::Tracer& tracer);

        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;

        bool Empty() const override;
        std::size_t Available() const override;

    private:
        infra::StreamReader& reader;
        services::Tracer& tracer;
    };

    class TracingInputStream
        : public infra::DataInputStream::WithReader<TracingStreamReader>
    {
    public:
        TracingInputStream(infra::DataInputStream& stream, services::Tracer& tracer);

        using DataInputStream::WithReader<TracingStreamReader>::WithReader;
    };
}

#endif
