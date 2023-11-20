#include "infra/stream/CountingOutputStream.hpp"

namespace infra
{
    CountingStreamWriter::CountingStreamWriter(infra::ByteRange saveStateStorage)
        : saveStateStorage(saveStateStorage)
    {}

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
        return processed - marker;
    }

    infra::ByteRange CountingStreamWriter::SaveState(std::size_t marker)
    {
        return saveStateStorage;
    }

    void CountingStreamWriter::RestoreState(infra::ByteRange range)
    {
        processed += saveStateStorage.size() - range.size();
    }

    infra::ByteRange CountingStreamWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }
}
