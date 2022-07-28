#include "infra/stream/BoundedDequeOutputStream.hpp"

namespace infra
{
    BoundedDequeStreamWriter::BoundedDequeStreamWriter(BoundedDeque<uint8_t>& deque)
        : deque(&deque)
    {}

    void BoundedDequeStreamWriter::Reset()
    {
        deque->clear();
    }

    void BoundedDequeStreamWriter::Reset(BoundedDeque<uint8_t>& newString)
    {
        deque = &newString;
    }

    void BoundedDequeStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        std::size_t spaceLeft = Available();
        bool spaceOk = range.size() <= spaceLeft;
        errorPolicy.ReportResult(spaceOk);
        range.shrink_from_back_to(spaceLeft);
        deque->insert(deque->end(), range.begin(), range.end());
    }

    std::size_t BoundedDequeStreamWriter::Available() const
    {
        return deque->max_size() - deque->size();
    }

    std::size_t BoundedDequeStreamWriter::ConstructSaveMarker() const
    {
        return deque->size();
    }

    std::size_t BoundedDequeStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return deque->size() - marker;
    }

    infra::ByteRange BoundedDequeStreamWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void BoundedDequeStreamWriter::RestoreState(infra::ByteRange range)
    {
        std::abort();
    }

    infra::ByteRange BoundedDequeStreamWriter::Overwrite(std::size_t marker)
    {
        std::abort();   // The requested range may be split in the deque
    }

    BoundedDequeOutputStream::BoundedDequeOutputStream(BoundedDeque<uint8_t>& storage)
        : DataOutputStream::WithWriter<BoundedDequeStreamWriter>(storage)
    {}

    BoundedDequeOutputStream::BoundedDequeOutputStream(BoundedDeque<uint8_t>& storage, const SoftFail&)
        : DataOutputStream::WithWriter<BoundedDequeStreamWriter>(storage, softFail)
    {}

    BoundedDequeOutputStream::BoundedDequeOutputStream(BoundedDeque<uint8_t>& storage, const NoFail&)
        : DataOutputStream::WithWriter<BoundedDequeStreamWriter>(storage, noFail)
    {}
}
