#ifndef INFRA_STD_VECTOR_INPUT_STREAM_HPP
#define INFRA_STD_VECTOR_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/WithStorage.hpp"
#include <cstdint>
#include <vector>

namespace infra
{
    class StdVectorInputStreamReader
        : public StreamReaderWithRewinding
    {
    public:
        using WithStorage = infra::WithStorage<StdVectorInputStreamReader, std::vector<uint8_t>>;

        explicit StdVectorInputStreamReader(const std::vector<uint8_t>& vector);

        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;
        virtual std::size_t ConstructSaveMarker() const override;
        virtual void Rewind(std::size_t marker) override;

    private:
        std::size_t offset = 0;
        const std::vector<uint8_t>& vector;
    };

    class StdVectorInputStream
        : public DataInputStream::WithReader<StdVectorInputStreamReader>
    {
    public:
        using WithStorage = infra::WithStorage<StdVectorInputStream, std::vector<uint8_t>>;

        explicit StdVectorInputStream(std::vector<uint8_t>& storage);
        StdVectorInputStream(std::vector<uint8_t>& storage, const SoftFail&);
        StdVectorInputStream(std::vector<uint8_t>& storage, const NoFail&);
    };
}

#endif
