#ifndef INFRA_STD_VECTOR_OUTPUT_STREAM_HPP
#define INFRA_STD_VECTOR_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/WithStorage.hpp"
#include <vector>

namespace infra
{
    class StdVectorOutputStreamWriter
        : public StreamWriter
    {
    public:
        using WithStorage = infra::WithStorage<StdVectorOutputStreamWriter, std::vector<uint8_t>>;

        explicit StdVectorOutputStreamWriter(std::vector<uint8_t>& vector, std::size_t saveSize = 1024);

    public:
        void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        infra::ByteRange SaveState(std::size_t marker) override;
        void RestoreState(infra::ByteRange range) override;
        infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        std::vector<uint8_t>& vector;
        std::size_t saveSize;
        std::vector<std::vector<uint8_t>> savedState;
    };

    class StdVectorOutputStream
        : public DataOutputStream::WithWriter<StdVectorOutputStreamWriter>
    {
    public:
        using WithStorage = infra::WithStorage<DataOutputStream::WithWriter<StdVectorOutputStreamWriter>, std::vector<uint8_t>>;

        explicit StdVectorOutputStream(std::vector<uint8_t>& vector);
        StdVectorOutputStream(std::vector<uint8_t>& vector, const SoftFail&);
        StdVectorOutputStream(std::vector<uint8_t>& vector, const NoFail&);
    };
}

#endif
