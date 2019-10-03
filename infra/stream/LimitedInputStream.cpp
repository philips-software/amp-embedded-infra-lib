#include "infra/stream/LimitedInputStream.hpp"

namespace infra
{
    LimitedStreamReader::LimitedStreamReader(StreamReader& input, uint32_t length)
        : input(input)
        , length(length)
    {}

    LimitedStreamReader::LimitedStreamReader(const LimitedStreamReader& other)
        : input(other.input)
        , length(other.length)
    {}

    void LimitedStreamReader::ResetLength(uint32_t newLength)
    {
        length = newLength;
    }

    void LimitedStreamReader::Extract(ByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(length >= range.size());
        range.shrink_from_back_to(length);
        length -= range.size();
        input.Extract(range, errorPolicy);
    }

    uint8_t LimitedStreamReader::Peek(StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(length != 0);

        if (length != 0)
            return input.Peek(errorPolicy);
        else
            return 0;
    }

    ConstByteRange LimitedStreamReader::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result = input.ExtractContiguousRange(std::min<std::size_t>(length, max));
        length -= result.size();
        return result;
    }

    ConstByteRange infra::LimitedStreamReader::PeekContiguousRange(std::size_t start)
    {
        return input.PeekContiguousRange(start);
    }

    bool LimitedStreamReader::Empty() const
    {
        return length == 0 || input.Empty();
    }

    std::size_t LimitedStreamReader::Available() const
    {
        return std::min<uint32_t>(length, input.Available());
    }

    LimitedStreamReaderWithRewinding::LimitedStreamReaderWithRewinding(StreamReaderWithRewinding& input, uint32_t length)
        : input(input)
        , length(length)
    {}

    LimitedStreamReaderWithRewinding::LimitedStreamReaderWithRewinding(const LimitedStreamReaderWithRewinding& other)
        : input(other.input)
        , length(other.length)
    {}

    void LimitedStreamReaderWithRewinding::ResetLength(uint32_t newLength)
    {
        length = newLength;
    }

    void LimitedStreamReaderWithRewinding::Extract(ByteRange range, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(length >= range.size());
        range.shrink_from_back_to(length);
        length -= range.size();
        input.Extract(range, errorPolicy);
    }

    uint8_t LimitedStreamReaderWithRewinding::Peek(StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(length != 0);

        if (length != 0)
            return input.Peek(errorPolicy);
        else
            return 0;
    }

    ConstByteRange LimitedStreamReaderWithRewinding::ExtractContiguousRange(std::size_t max)
    {
        ConstByteRange result = input.ExtractContiguousRange(std::min<std::size_t>(length, max));
        length -= result.size();
        return result;
    }

    ConstByteRange infra::LimitedStreamReaderWithRewinding::PeekContiguousRange(std::size_t start)
    {
        return input.PeekContiguousRange(start);
    }

    bool LimitedStreamReaderWithRewinding::Empty() const
    {
        return length == 0 || input.Empty();
    }

    std::size_t LimitedStreamReaderWithRewinding::Available() const
    {
        return std::min<uint32_t>(length, input.Available());
    }

    std::size_t LimitedStreamReaderWithRewinding::ConstructSaveMarker() const
    {
        return input.ConstructSaveMarker();
    }

    void LimitedStreamReaderWithRewinding::Rewind(std::size_t marker)
    {
        auto now = input.ConstructSaveMarker();
        input.Rewind(marker);

        length += now - marker;
    }
}
