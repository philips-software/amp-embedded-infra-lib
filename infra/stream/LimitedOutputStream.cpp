#include "infra/stream/LimitedOutputStream.hpp"

namespace infra
{
    LimitedStreamWriter::LimitedStreamWriter(StreamWriter& output, uint32_t length)
        : output(output)
        , length(length)
    {}

    LimitedStreamWriter::LimitedStreamWriter(const LimitedStreamWriter& other)
        : output(other.output)
        , length(other.length)
    {}

    void LimitedStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(length >= range.size());
        range.shrink_from_back_to(length);
        length -= range.size();
        output.Insert(range, errorPolicy);
    }

    std::size_t LimitedStreamWriter::Available() const
    {
        return std::min<std::size_t>(length, output.Available());
    }

    std::size_t LimitedStreamWriter::ConstructSaveMarker() const
    {
        return output.ConstructSaveMarker();
    }

    std::size_t LimitedStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return output.GetProcessedBytesSince(marker);
    }

    infra::ByteRange LimitedStreamWriter::SaveState(std::size_t marker)
    {
        return output.SaveState(marker);
    }

    void LimitedStreamWriter::RestoreState(infra::ByteRange range)
    {
        output.RestoreState(range);
    }

    infra::ByteRange LimitedStreamWriter::Overwrite(std::size_t marker)
    {
        length += output.ConstructSaveMarker() - marker;
        return output.Overwrite(marker);
    }
}
