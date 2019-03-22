#include "infra/syntax/JsonInputStream.hpp"

namespace infra
{
    JsonInputStreamReader::JsonInputStreamReader(JsonString string)
        : string(string)
        , position(string.begin())
    {}

    void JsonInputStreamReader::Extract(ByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(Available() >= range.size());
        range.shrink_from_back_to(Available());
        auto newPosition = std::next(position, range.size());
        std::copy(position, newPosition, range.begin());
        position = newPosition;
    }

    uint8_t JsonInputStreamReader::Peek(StreamErrorPolicy& errorPolicy)
    {
        if (Empty())
        {
            errorPolicy.ReportResult(false);
            return 0;
        }
        else
        {
            errorPolicy.ReportResult(true);
            return static_cast<uint8_t>(*position);
        }
    }

    ConstByteRange JsonInputStreamReader::ExtractContiguousRange(std::size_t max)
    {
        std::abort(); // Not implemented
    }

    ConstByteRange infra::JsonInputStreamReader::PeekContiguousRange(std::size_t start)
    {
        std::abort(); // Not implemented
    }

    bool JsonInputStreamReader::Empty() const
    {
        return position == string.end();
    }

    std::size_t JsonInputStreamReader::Available() const
    {
        return std::distance(position, string.end());
    }

    JsonInputStream::JsonInputStream(JsonString storage)
        : TextInputStream::WithReader<JsonInputStreamReader>(storage)
    {}

    JsonInputStream::JsonInputStream(JsonString storage, const SoftFail&)
        : TextInputStream::WithReader<JsonInputStreamReader>(storage, softFail)
    {}

    JsonInputStream::JsonInputStream(JsonString storage, const NoFail&)
        : TextInputStream::WithReader<JsonInputStreamReader>(storage, noFail)
    {}
}
