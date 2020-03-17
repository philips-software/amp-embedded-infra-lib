#include "infra/stream/StdStringInputStream.hpp"

namespace infra
{
    StdStringInputStreamReader::StdStringInputStreamReader(const std::string& string)
        : string(string)
    {}

    void StdStringInputStreamReader::Extract(ByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(offset + range.size() <= string.size());
        range.shrink_from_back_to(string.size() - offset);
        std::copy(string.begin() + offset, string.begin() + offset + range.size(), range.begin());
        offset += range.size();
    }

    uint8_t StdStringInputStreamReader::Peek(StreamErrorPolicy& errorPolicy)
    {
        if (offset == string.size())
        {
            errorPolicy.ReportResult(false);
            return 0;
        }
        else
        {
            errorPolicy.ReportResult(true);
            return static_cast<uint8_t>(string.begin()[offset]);
        }
    }

    ConstByteRange StdStringInputStreamReader::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result(reinterpret_cast<const uint8_t*>(string.data()) + offset, reinterpret_cast<const uint8_t*>(string.data()) + string.size());
        result.shrink_from_back_to(max);
        offset += result.size();
        return result;
    }

    ConstByteRange StdStringInputStreamReader::PeekContiguousRange(std::size_t start)
    {
        ConstByteRange result(reinterpret_cast<const uint8_t*>(string.data()) + offset + start, reinterpret_cast<const uint8_t*>(string.size()));

        return result;
    }

    bool StdStringInputStreamReader::Empty() const
    {
        return offset == string.size();
    }

    std::size_t StdStringInputStreamReader::Available() const
    {
        return string.size() - offset;
    }

    StdStringInputStream::StdStringInputStream(const std::string& storage)
        : TextInputStream::WithReader<StdStringInputStreamReader>(storage)
    {}

    StdStringInputStream::StdStringInputStream(const std::string& storage, const SoftFail&)
        : TextInputStream::WithReader<StdStringInputStreamReader>(storage, softFail)
    {}

    StdStringInputStream::StdStringInputStream(const std::string& storage, const NoFail&)
        : TextInputStream::WithReader<StdStringInputStreamReader>(storage, noFail)
    {}
}
