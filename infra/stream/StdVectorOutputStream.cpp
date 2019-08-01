#include "infra/stream/StdVectorOutputStream.hpp"

namespace infra
{
    StdVectorOutputStreamWriter::StdVectorOutputStreamWriter(std::vector<uint8_t>& vector, std::size_t saveSize)
        : vector(vector)
        , saveSize(saveSize)
    {}

    void StdVectorOutputStreamWriter::Insert(ConstByteRange dataRange, StreamErrorPolicy& errorPolicy)
    {
        errorPolicy.ReportResult(true);
        vector.insert(vector.end(), dataRange.begin(), dataRange.end());
    }

    std::size_t StdVectorOutputStreamWriter::Available() const
    {
        return vector.max_size() - vector.size();
    }

    std::size_t StdVectorOutputStreamWriter::ConstructSaveMarker() const
    {
        return vector.size();
    }

    std::size_t StdVectorOutputStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return static_cast<std::size_t>(vector.size() - marker);
    }

    infra::ByteRange StdVectorOutputStreamWriter::SaveState(std::size_t marker)
    {
        savedState.push_back(std::vector<uint8_t>(vector.begin() + marker, vector.end()));

        vector.erase(vector.begin() + marker, vector.end());
        vector.insert(vector.end(), saveSize, 0);

        return infra::ByteRange(vector.data() + marker, vector.data() + vector.size());
    }

    void StdVectorOutputStreamWriter::RestoreState(infra::ByteRange range)
    {
        vector.erase(vector.end() - range.size(), vector.end());
        vector.insert(vector.end(), savedState.back().begin(), savedState.back().end());
        savedState.pop_back();
    }

    infra::ByteRange StdVectorOutputStreamWriter::Overwrite(std::size_t marker)
    {
        return infra::ByteRange(vector.data() + marker, vector.data() + vector.size());
    }

    StdVectorOutputStream::StdVectorOutputStream(std::vector<uint8_t>& vector)
        : DataOutputStream::WithWriter<StdVectorOutputStreamWriter>(vector)
    {}

    StdVectorOutputStream::StdVectorOutputStream(std::vector<uint8_t>& vector, const SoftFail&)
        : DataOutputStream::WithWriter<StdVectorOutputStreamWriter>(vector, softFail)
    {}

    StdVectorOutputStream::StdVectorOutputStream(std::vector<uint8_t>& vector, const NoFail&)
        : DataOutputStream::WithWriter<StdVectorOutputStreamWriter>(vector, noFail)
    {}
}
