#include "infra/stream/BoundedVectorOutputStream.hpp"

namespace infra
{
    BoundedVectorStreamWriter::BoundedVectorStreamWriter(BoundedVector<uint8_t>& vector)
        : vector(&vector)
    {}

    void BoundedVectorStreamWriter::Reset()
    {
        vector->clear();
    }

    void BoundedVectorStreamWriter::Reset(BoundedVector<uint8_t>& newString)
    {
        vector = &newString;
    }

    void BoundedVectorStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        std::size_t spaceLeft = Available();
        bool spaceOk = range.size() <= spaceLeft;
        errorPolicy.ReportResult(spaceOk);
        range.shrink_from_back_to(spaceLeft);
        vector->insert(vector->end(), range.begin(), range.end());
    }

    std::size_t BoundedVectorStreamWriter::Available() const
    {
        return vector->max_size() - vector->size();
    }

    std::size_t BoundedVectorStreamWriter::ConstructSaveMarker() const
    {
        return vector->size();
    }

    std::size_t BoundedVectorStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return vector->size() - marker;
    }

    infra::ByteRange BoundedVectorStreamWriter::SaveState(std::size_t marker)
    {
        auto copyBegin = vector->begin() + marker;
        auto copyEnd = vector->end();
        vector->resize(vector->max_size());
        std::copy_backward(copyBegin, copyEnd, vector->end());

        return infra::ByteRange(copyBegin, vector->end() - std::distance(copyBegin, copyEnd));
    }

    void BoundedVectorStreamWriter::RestoreState(infra::ByteRange range)
    {
        std::copy(range.end(), vector->end(), range.begin());
        vector->resize(vector->size() - range.size());
    }

    infra::ByteRange BoundedVectorStreamWriter::Overwrite(std::size_t marker)
    {
        return infra::DiscardHead(infra::ReinterpretCastByteRange(infra::MakeRange(*vector)), marker);
    }

    BoundedVectorOutputStream::BoundedVectorOutputStream(BoundedVector<uint8_t>& storage)
        : TextOutputStream::WithWriter<BoundedVectorStreamWriter>(storage)
    {}

    BoundedVectorOutputStream::BoundedVectorOutputStream(BoundedVector<uint8_t>& storage, const SoftFail&)
        : TextOutputStream::WithWriter<BoundedVectorStreamWriter>(storage, softFail)
    {}

    BoundedVectorOutputStream::BoundedVectorOutputStream(BoundedVector<uint8_t>& storage, const NoFail&)
        : TextOutputStream::WithWriter<BoundedVectorStreamWriter>(storage, noFail)
    {}
}
