#include "protobuf/echo/BufferingStreamWriter.hpp"

namespace services
{
    BufferingStreamWriter::BufferingStreamWriter(infra::BoundedDeque<uint8_t>& buffer, infra::ByteRange outputData)
        : buffer(buffer)
        , outputData(outputData)
    {
        LoadRemainder();
    }

    void BufferingStreamWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        if (index < outputData.size())
        {
            auto first = infra::Head(range, outputData.size() - index);
            infra::Copy(first, infra::Head(infra::DiscardHead(outputData, index), first.size()));
            index += first.size();
            range.pop_front(first.size());
        }

        buffer.insert(buffer.end(), range.begin(), range.end());
        index += range.size();
    }

    std::size_t BufferingStreamWriter::Available() const
    {
        return outputData.size() + buffer.size() - index;
    }

    std::size_t BufferingStreamWriter::ConstructSaveMarker() const
    {
        return index;
    }

    std::size_t BufferingStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return index - marker;
    }

    infra::ByteRange BufferingStreamWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void BufferingStreamWriter::RestoreState(infra::ByteRange range)
    {
        std::abort();
    }

    infra::ByteRange BufferingStreamWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }

    void BufferingStreamWriter::LoadRemainder()
    {
        auto from = infra::Head(buffer.contiguous_range(buffer.begin()), outputData.size());
        infra::Copy(from, infra::Head(outputData, from.size()));
        buffer.erase(buffer.begin(), buffer.begin() + from.size());
        outputData.pop_front(from.size());
        from = infra::Head(buffer.contiguous_range(buffer.begin()), outputData.size());
        infra::Copy(from, infra::Head(outputData, buffer.size()));
        buffer.clear();
        outputData.pop_front(from.size());

        index = 0;
    }
}
