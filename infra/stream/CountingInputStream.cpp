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
}
