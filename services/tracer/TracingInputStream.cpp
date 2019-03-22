#include "services/tracer/TracingInputStream.hpp"

namespace services
{
    TracingStreamReader::TracingStreamReader(infra::StreamReader& reader, services::Tracer& tracer)
        : reader(reader)
        , tracer(tracer)
    {}

    void TracingStreamReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        reader.Extract(range, errorPolicy);
        tracer.Trace() << "" << infra::AsHex(range);
    }

    uint8_t TracingStreamReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        return reader.Peek(errorPolicy);
    }

    infra::ConstByteRange TracingStreamReader::ExtractContiguousRange(std::size_t max)
    {
        infra::ConstByteRange result = reader.ExtractContiguousRange(max);
        tracer.Trace() << "" << infra::AsHex(result);
        return result;
    }

    infra::ConstByteRange services::TracingStreamReader::PeekContiguousRange(std::size_t start)
    {
        infra::ConstByteRange result = reader.PeekContiguousRange(start);
        tracer.Trace() << "" << infra::AsHex(result);
        return result;
    }

    bool TracingStreamReader::Empty() const
    {
        return reader.Empty();
    }

    std::size_t TracingStreamReader::Available() const
    {
        return reader.Available();
    }

    TracingInputStream::TracingInputStream(infra::DataInputStream& stream, services::Tracer& tracer)
        : infra::DataInputStream::WithReader<TracingStreamReader>(stream.Reader(), tracer)
    {}
}
