#include "infra/stream/CountingInputStream.hpp"

namespace infra
{
    CountingStreamReader::CountingStreamReader(infra::StreamReader& reader)
        : reader(reader)
    {}

    uint32_t CountingStreamReader::TotalRead() const
    {
        return totalRead;
    }

    void CountingStreamReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        otherErrorPolicy = &errorPolicy;
        ok = true;
        reader.Extract(range, *this);

        if (ok)
            totalRead += range.size();
        otherErrorPolicy = nullptr;
    }

    uint8_t CountingStreamReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        return reader.Peek(errorPolicy);
    }

    infra::ConstByteRange CountingStreamReader::ExtractContiguousRange(std::size_t max)
    {
        auto result = reader.ExtractContiguousRange(max);
        totalRead += result.size();
        return result;
    }

    infra::ConstByteRange CountingStreamReader::PeekContiguousRange(std::size_t start)
    {
        return reader.PeekContiguousRange(start);
    }

    bool CountingStreamReader::Empty() const
    {
        return reader.Empty();
    }

    std::size_t CountingStreamReader::Available() const
    {
        return reader.Available();
    }

    bool CountingStreamReader::Failed() const
    {
        return otherErrorPolicy->Failed();
    }

    void CountingStreamReader::ReportResult(bool ok)
    {
        otherErrorPolicy->ReportResult(ok);
        this->ok = this->ok && ok;
    }

    CountingStreamReaderWithRewinding::CountingStreamReaderWithRewinding(infra::StreamReaderWithRewinding& reader)
        : reader(reader)
    {}

    uint32_t CountingStreamReaderWithRewinding::TotalRead() const
    {
        return totalRead;
    }

    void CountingStreamReaderWithRewinding::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        otherErrorPolicy = &errorPolicy;
        ok = true;
        reader.Extract(range, *this);

        if (ok)
            totalRead += range.size();
        otherErrorPolicy = nullptr;
    }

    uint8_t CountingStreamReaderWithRewinding::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        return reader.Peek(errorPolicy);
    }

    infra::ConstByteRange CountingStreamReaderWithRewinding::ExtractContiguousRange(std::size_t max)
    {
        auto result = reader.ExtractContiguousRange(max);
        totalRead += result.size();
        return result;
    }

    infra::ConstByteRange CountingStreamReaderWithRewinding::PeekContiguousRange(std::size_t start)
    {
        return reader.PeekContiguousRange(start);
    }

    bool CountingStreamReaderWithRewinding::Empty() const
    {
        return reader.Empty();
    }

    std::size_t CountingStreamReaderWithRewinding::Available() const
    {
        return reader.Available();
    }

    std::size_t CountingStreamReaderWithRewinding::ConstructSaveMarker() const
    {
        return reader.ConstructSaveMarker();
    }

    void CountingStreamReaderWithRewinding::Rewind(std::size_t marker)
    {
        totalRead -= reader.ConstructSaveMarker() - marker;
        reader.Rewind(marker);
    }

    bool CountingStreamReaderWithRewinding::Failed() const
    {
        return otherErrorPolicy->Failed();
    }

    void CountingStreamReaderWithRewinding::ReportResult(bool ok)
    {
        otherErrorPolicy->ReportResult(ok);
        this->ok = this->ok && ok;
    }
}
