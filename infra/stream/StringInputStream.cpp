#include "infra/stream/StringInputStream.hpp"

namespace infra
{
    StringInputStreamReader::StringInputStreamReader(BoundedConstString string)
        : string(string)
    {}

    void StringInputStreamReader::Extract(ByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(offset + range.size() <= string.size());
        range.shrink_from_back_to(string.size() - offset);
        std::copy(string.begin() + offset, string.begin() + offset + range.size(), range.begin());
        offset += range.size();
    }

    uint8_t StringInputStreamReader::Peek(StreamErrorPolicy& errorPolicy)
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

    ConstByteRange StringInputStreamReader::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result(reinterpret_cast<const uint8_t*>(string.data()) + offset, reinterpret_cast<const uint8_t*>(string.data()) + string.size());
        result.shrink_from_back_to(max);
        offset += result.size();
        return result;
    }

    ConstByteRange infra::StringInputStreamReader::PeekContiguousRange(std::size_t start)
    {
        ConstByteRange result(reinterpret_cast<const uint8_t*>(string.data()) + offset + start, reinterpret_cast<const uint8_t*>(string.data() + string.size()));
        return result;
    }

    bool StringInputStreamReader::Empty() const
    {
        return offset == string.size();
    }

    std::size_t StringInputStreamReader::Available() const
    {
        return string.size() - offset;
    }

    StringInputStream::StringInputStream(BoundedConstString storage)
        : TextInputStream::WithReader<StringInputStreamReader>(storage)
    {}

    StringInputStream::StringInputStream(BoundedConstString storage, const SoftFail&)
        : TextInputStream::WithReader<StringInputStreamReader>(storage, softFail)
    {}

    StringInputStream::StringInputStream(BoundedConstString storage, const NoFail&)
        : TextInputStream::WithReader<StringInputStreamReader>(storage, noFail)
    {}
}
