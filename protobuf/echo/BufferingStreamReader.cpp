#include "protobuf/echo/BufferingStreamReader.hpp"

namespace services
{
    BufferingStreamReader::BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::ConstByteRange inputData)
        : buffer(buffer)
        , inputData(inputData)
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
            auto from = infra::Head(infra::DiscardHead(inputData, dataIndex), range.size());
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

        auto dataIndex = index - buffer.size();
        auto from = infra::Head(infra::DiscardHead(inputData, dataIndex), max);
        index += from.size();
        return from;
    }

    infra::ConstByteRange BufferingStreamReader::PeekContiguousRange(std::size_t start)
    {
        if (index + start < buffer.size())
        {
            auto from = buffer.contiguous_range(buffer.begin() + index + start);
            return from;
        }

        auto dataIndex = index + start - buffer.size();
        auto from = infra::DiscardHead(inputData, dataIndex);
        return from;
    }

    bool BufferingStreamReader::Empty() const
    {
        return Available() == 0;
    }

    std::size_t BufferingStreamReader::Available() const
    {
        return buffer.size() + inputData.size() - index;
    }

    std::size_t BufferingStreamReader::ConstructSaveMarker() const
    {
        return index;
    }

    void BufferingStreamReader::Rewind(std::size_t marker)
    {
        index = marker;
    }

    void BufferingStreamReader::StoreRemainder()
    {
        std::size_t bufferDecrease = std::min(buffer.size(), index);
        buffer.erase(buffer.begin(), buffer.begin() + bufferDecrease);
        buffer.insert(buffer.end(), inputData.begin() + index - bufferDecrease, inputData.end());
    }
}
