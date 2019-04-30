#include "infra/stream/StdVectorInputStream.hpp"

namespace infra
{
    StdVectorInputStreamReader::StdVectorInputStreamReader(const std::vector<uint8_t>& vector)
        : vector(vector)
    {}

    void StdVectorInputStreamReader::Extract(ByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(offset + range.size() <= vector.size());
        range.shrink_from_back_to(vector.size() - offset);
        std::copy(vector.begin() + offset, vector.begin() + offset + range.size(), range.begin());
        offset += range.size();
    }

    uint8_t StdVectorInputStreamReader::Peek(StreamErrorPolicy& errorPolicy)
    {
        if (offset == vector.size())
        {
            errorPolicy.ReportResult(false);
            return 0;
        }
        else
        {
            errorPolicy.ReportResult(true);
            return static_cast<uint8_t>(vector.begin()[offset]);
        }
    }

    ConstByteRange StdVectorInputStreamReader::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result(reinterpret_cast<const uint8_t*>(vector.data()) + offset, reinterpret_cast<const uint8_t*>(vector.data()) + vector.size());
        result.shrink_from_back_to(max);
        offset += result.size();
        return result;
    }

    ConstByteRange StdVectorInputStreamReader::PeekContiguousRange(std::size_t start)
    {
        ConstByteRange result(reinterpret_cast<const uint8_t*>(vector.data()) + offset + start, reinterpret_cast<const uint8_t*>(vector.size()));

        return result;
    }

    bool StdVectorInputStreamReader::Empty() const
    {
        return offset == vector.size();
    }

    std::size_t StdVectorInputStreamReader::Available() const
    {
        return vector.size() - offset;
    }

    StdVectorInputStream::StdVectorInputStream(std::vector<uint8_t>& storage)
        : DataInputStream::WithReader<StdVectorInputStreamReader>(storage)
    {}

    StdVectorInputStream::StdVectorInputStream(std::vector<uint8_t>& storage, const SoftFail&)
        : DataInputStream::WithReader<StdVectorInputStreamReader>(storage, softFail)
    {}

    StdVectorInputStream::StdVectorInputStream(std::vector<uint8_t>& storage, const NoFail&)
        : DataInputStream::WithReader<StdVectorInputStreamReader>(storage, noFail)
    {}
}
