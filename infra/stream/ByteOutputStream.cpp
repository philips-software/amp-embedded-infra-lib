#include "infra/stream/ByteOutputStream.hpp"

namespace infra
{
    ByteOutputStreamWriter::ByteOutputStreamWriter(ByteRange streamRange)
        : streamRange(streamRange)
    {}

    ByteRange ByteOutputStreamWriter::Processed() const
    {
        return MakeRange(streamRange.begin(), streamRange.begin() + offset);
    }

    ByteRange ByteOutputStreamWriter::Remaining() const
    {
        return MakeRange(streamRange.begin() + offset, streamRange.end());
    }

    void ByteOutputStreamWriter::Reset()
    {
        offset = 0;
    }

    void ByteOutputStreamWriter::Reset(ByteRange range)
    {
        offset = 0;
        streamRange = range;
    }

    void ByteOutputStreamWriter::Insert(ConstByteRange dataRange, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(dataRange.size() <= streamRange.size() - offset);
        dataRange.shrink_from_back_to(streamRange.size() - offset);
        std::copy(dataRange.begin(), dataRange.begin() + dataRange.size(), streamRange.begin() + offset);
        offset += dataRange.size();
    }

    std::size_t ByteOutputStreamWriter::Available() const
    {
        return streamRange.size() - offset;
    }

    std::size_t ByteOutputStreamWriter::ConstructSaveMarker() const
    {
        return offset;
    }

    std::size_t ByteOutputStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return offset - marker;
    }

    infra::ByteRange ByteOutputStreamWriter::SaveState(std::size_t marker)
    {
        uint8_t* copyBegin = streamRange.begin() + marker;
        uint8_t* copyEnd = streamRange.begin() + offset;
        std::copy_backward(copyBegin, copyEnd, streamRange.end());

        return infra::ByteRange(copyBegin, streamRange.end() - std::distance(copyBegin, copyEnd));
    }

    void ByteOutputStreamWriter::RestoreState(infra::ByteRange range)
    {
        std::copy(range.end(), streamRange.end(), range.begin());
        offset += std::distance(streamRange.begin() + offset, range.begin()) + std::distance(range.end(), streamRange.end());
    }

    infra::ByteRange ByteOutputStreamWriter::Overwrite(std::size_t marker)
    {
        return infra::DiscardHead(streamRange, marker);
    }

    ByteOutputStream::ByteOutputStream(ByteRange storage)
        : DataOutputStream::WithWriter<ByteOutputStreamWriter>(storage)
    {}

    ByteOutputStream::ByteOutputStream(ByteRange storage, const SoftFail&)
        : DataOutputStream::WithWriter<ByteOutputStreamWriter>(storage, softFail)
    {}

    ByteOutputStream::ByteOutputStream(ByteRange storage, const NoFail&)
        : DataOutputStream::WithWriter<ByteOutputStreamWriter>(storage, noFail)
    {}
}
