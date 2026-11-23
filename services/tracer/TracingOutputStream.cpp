#include "services/tracer/TracingOutputStream.hpp"

namespace services
{
    TracingStreamWriter::TracingStreamWriter(infra::StreamWriter& writer, services::Tracer& tracer)
        : writer(writer)
        , tracer(tracer)
    {}

    void TracingStreamWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        writer.Insert(range, errorPolicy);
        tracer.Continue() << infra::ByteRangeAsString(range);
    }

    std::size_t TracingStreamWriter::Available() const
    {
        return writer.Available();
    }

    std::size_t TracingStreamWriter::ConstructSaveMarker() const
    {
        return writer.ConstructSaveMarker();
    }

    std::size_t TracingStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return writer.GetProcessedBytesSince(marker);
    }

    infra::ByteRange TracingStreamWriter::SaveState(std::size_t marker)
    {
        return writer.SaveState(marker);
    }

    void TracingStreamWriter::RestoreState(infra::ByteRange range)
    {
        writer.RestoreState(range);
    }

    infra::ByteRange TracingStreamWriter::Overwrite(std::size_t marker)
    {
        return writer.Overwrite(marker);
    }

    TracingAsciiStreamWriter::TracingAsciiStreamWriter(infra::StreamWriter& writer, services::Tracer& tracer)
        : TracingStreamWriter(writer, tracer)
        , writer(writer)
        , tracer(tracer)
    {}

    void TracingAsciiStreamWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        writer.Insert(range, errorPolicy);
        tracer.Continue() << infra::AsAscii(range);
    }
}
