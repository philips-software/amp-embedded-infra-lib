#include "infra/stream/BufferingStreamReader.hpp"

namespace infra
{
    BufferingStreamReader::BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::StreamReaderWithRewinding& input)
        : buffer(buffer)
        , input(input)
    {}

    BufferingStreamReader::~BufferingStreamReader()
    {
        StoreRemainder();
    }

    void BufferingStreamReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        if (index != buffer.size())
        {
            auto from = infra::Head(buffer.contiguous_range(buffer.begin() + index), range.size());
            infra::Copy(from, infra::Head(range, from.size()));
            range.pop_front(from.size());
            index += from.size();

            // Perhaps the deque just wrapped around, try once more
            from = infra::Head(buffer.contiguous_range(buffer.begin() + index), range.size());
            infra::Copy(from, infra::Head(range, from.size()));
            range.pop_front(from.size());
            index += from.size();
        }

        if (!range.empty())
        {
            auto dataIndex = index - buffer.size();
            auto from = input.ExtractContiguousRange(range.size());
            infra::Copy(from, infra::Head(range, from.size()));
            range.pop_front(from.size());
            index += from.size();
        }

        errorPolicy.ReportResult(range.empty());
    }

    uint8_t BufferingStreamReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        auto range = PeekContiguousRange(0);

        errorPolicy.ReportResult(!range.empty());

        if (range.empty())
            return 0;
        else
            return range.front();
    }

    infra::ConstByteRange BufferingStreamReader::ExtractContiguousRange(std::size_t max)
    {
        if (index < buffer.size())
        {
            auto from = infra::Head(buffer.contiguous_range(buffer.begin() + index), max);
            index += from.size();
            return from;
        }

        auto from = input.ExtractContiguousRange(max);
        return from;
    }

    infra::ConstByteRange BufferingStreamReader::PeekContiguousRange(std::size_t start)
    {
        if (index + start < buffer.size())
            return buffer.contiguous_range(buffer.begin() + index + start);

        return input.PeekContiguousRange(index + start - buffer.size());
    }

    bool BufferingStreamReader::Empty() const
    {
        return Available() == 0;
    }

    std::size_t BufferingStreamReader::Available() const
    {
        return buffer.size() + input.Available();
    }

    std::size_t BufferingStreamReader::ConstructSaveMarker() const
    {
        return index;
    }

    void BufferingStreamReader::Rewind(std::size_t marker)
    {
        if (index > buffer.size())
        {
            auto rewindAmount = std::min(index - marker, index - buffer.size());
            input.Rewind(input.ConstructSaveMarker() - rewindAmount);
            index -= rewindAmount;
        }

        if (marker < buffer.size())
            index = marker;
    }

    void BufferingStreamReader::StoreRemainder()
    {
        std::size_t bufferDecrease = std::min(buffer.size(), index);
        buffer.erase(buffer.begin(), buffer.begin() + bufferDecrease);
        while (!input.Empty())
        {
            auto range = input.ExtractContiguousRange(std::numeric_limits<std::size_t>::max());
            buffer.insert(buffer.end(), range.begin(), range.end());
        }
    }
}
