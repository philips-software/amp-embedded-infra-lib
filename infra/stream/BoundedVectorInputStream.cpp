#include "infra/stream/BoundedVectorInputStream.hpp"

namespace infra
{
    BoundedVectorInputStreamReader::BoundedVectorInputStreamReader(infra::BoundedVector<uint8_t>& container)
        : container(container)
    {}

    std::size_t BoundedVectorInputStreamReader::Processed() const
    {
        return offset;
    }

    void BoundedVectorInputStreamReader::Extract(ByteRange dataRange, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(dataRange.size() <= container.size() - offset);
        dataRange.shrink_from_back_to(container.size());

        ConstByteRange from = infra::Head(PeekContiguousRange(0), dataRange.size());
        infra::Copy(from, infra::Head(dataRange, from.size()));
        offset += from.size();
    }

    uint8_t BoundedVectorInputStreamReader::Peek(StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(!Empty());
        if (!Empty())
            return container[offset];
        else
            return 0;
    }

    ConstByteRange BoundedVectorInputStreamReader::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result = infra::Head(PeekContiguousRange(0), max);
        offset += result.size();
        return result;
    }

    ConstByteRange infra::BoundedVectorInputStreamReader::PeekContiguousRange(std::size_t start)
    {
        return{ container.begin() + offset + start, container.end() };
    }

    bool BoundedVectorInputStreamReader::Empty() const
    {  
        return offset == container.size();
    }

    std::size_t BoundedVectorInputStreamReader::Available() const
    {
        return container.size() - offset;
    }

    std::size_t  BoundedVectorInputStreamReader::ConstructSaveMarker() const
    {
        return offset;
    }

    void BoundedVectorInputStreamReader::Rewind(std::size_t marker)
    {
        offset = marker;
    }

    BoundedVectorInputStream::BoundedVectorInputStream(infra::BoundedVector<uint8_t>& storage)
        : DataInputStream::WithReader<BoundedVectorInputStreamReader>(storage)
    {}

    BoundedVectorInputStream::BoundedVectorInputStream(infra::BoundedVector<uint8_t>& storage, const SoftFail&)
        : DataInputStream::WithReader<BoundedVectorInputStreamReader>(storage, softFail)
    {}

    BoundedVectorInputStream::BoundedVectorInputStream(infra::BoundedVector<uint8_t>& storage, const NoFail&)
        : DataInputStream::WithReader<BoundedVectorInputStreamReader>(storage, noFail)
    {}
}
