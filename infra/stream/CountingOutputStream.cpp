#include "infra/stream/CountingOutputStream.hpp"

namespace infra
{
    std::size_t CountingStreamWriter::Processed() const
    {
        return processed;
    }

    void CountingStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        processed += range.size();
    }

    std::size_t CountingStreamWriter::Available() const
    {
        return std::numeric_limits<std::size_t>::max();
    }

    std::size_t CountingStreamWriter::ConstructSaveMarker() const
    {
        return processed;
    }

    std::size_t CountingStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return marker - processed;
    }

    infra::ByteRange CountingStreamWriter::SaveState(std::size_t marker)
    {
        return infra::ByteRange();
    }

    void CountingStreamWriter::RestoreState(infra::ByteRange range)
    {}

    infra::ByteRange CountingStreamWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }
}
