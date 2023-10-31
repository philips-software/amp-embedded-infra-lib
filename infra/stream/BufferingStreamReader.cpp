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
            Read(infra::Head(buffer.contiguous_range(buffer.begin() + index), range.size()), range);
            // Perhaps the deque just wrapped around, try once more
            Read(infra::Head(buffer.contiguous_range(buffer.begin() + index), range.size()), range);
        }

        if (!range.empty())
            Read(input.ExtractContiguousRange(range.size()), range);

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

        return input.ExtractContiguousRange(max);
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

    void BufferingStreamReader::Read(infra::ConstByteRange from, infra::ByteRange& to)
    {
        infra::Copy(from, infra::Head(to, from.size()));
        to.pop_front(from.size());
        index += from.size();
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
