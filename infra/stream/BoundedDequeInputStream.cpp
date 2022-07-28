#include "infra/stream/BoundedDequeInputStream.hpp"

namespace infra
{
    BoundedDequeInputStreamReader::BoundedDequeInputStreamReader(infra::BoundedDeque<uint8_t>& container)
        : container(container)
    {}

    std::size_t BoundedDequeInputStreamReader::Processed() const
    {
        return offset;
    }

    void BoundedDequeInputStreamReader::Extract(ByteRange dataRange, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(dataRange.size() <= container.size() - offset);
        while (!dataRange.empty() && container.size() != offset)
        {
            ConstByteRange from = infra::Head(container.contiguous_range(container.begin() + offset), dataRange.size());
            infra::Copy(from, infra::Head(dataRange, from.size()));
            dataRange.pop_front(from.size());
            offset += from.size();
        }
    }

    uint8_t BoundedDequeInputStreamReader::Peek(StreamErrorPolicy& errorPolicy)
    {
        auto savedOffset = offset;
        uint8_t element = 0;
        Extract(MakeByteRange(element), errorPolicy);
        offset = savedOffset;
        return element;
    }

    ConstByteRange BoundedDequeInputStreamReader::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result = infra::Head(container.contiguous_range(container.begin() + offset), max);
        offset += result.size();
        return result;
    }

    ConstByteRange infra::BoundedDequeInputStreamReader::PeekContiguousRange(std::size_t start)
    {
        return container.contiguous_range(container.begin() + offset + start);
    }

    bool BoundedDequeInputStreamReader::Empty() const
    {  
        return offset == container.size();
    }

    std::size_t BoundedDequeInputStreamReader::Available() const
    {
        return container.size() - offset;
    }

    std::size_t  BoundedDequeInputStreamReader::ConstructSaveMarker() const
    {
        return offset;
    }

    void BoundedDequeInputStreamReader::Rewind(std::size_t marker)
    {
        offset = marker;
    }

    BoundedDequeInputStream::BoundedDequeInputStream(infra::BoundedDeque<uint8_t>& storage)
        : DataInputStream::WithReader<BoundedDequeInputStreamReader>(storage)
    {}

    BoundedDequeInputStream::BoundedDequeInputStream(infra::BoundedDeque<uint8_t>& storage, const SoftFail&)
        : DataInputStream::WithReader<BoundedDequeInputStreamReader>(storage, softFail)
    {}

    BoundedDequeInputStream::BoundedDequeInputStream(infra::BoundedDeque<uint8_t>& storage, const NoFail&)
        : DataInputStream::WithReader<BoundedDequeInputStreamReader>(storage, noFail)
    {}
}
