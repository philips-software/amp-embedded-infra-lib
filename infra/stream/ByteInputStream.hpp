#ifndef INFRA_BYTE_INPUT_STREAM_HPP
#define INFRA_BYTE_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include <array>

namespace infra
{
    class ByteInputStreamReader
        : public StreamReaderWithRewinding
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<ByteInputStreamReader, std::array<uint8_t, Max>>;

        explicit ByteInputStreamReader(ConstByteRange range);

        void ResetRange(ConstByteRange newRange);

        ConstByteRange Processed() const; // Invariant: Processed() ++ Remaining() == range
        ConstByteRange Remaining() const;

    public:
        void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        ConstByteRange ExtractContiguousRange(std::size_t max) override;
        ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        void Rewind(std::size_t marker) override;

    private:
        ConstByteRange range;
        std::size_t offset = 0;
    };

    class ByteInputStream
        : public DataInputStream::WithReader<ByteInputStreamReader>
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<DataInputStream::WithReader<ByteInputStreamReader>, std::array<uint8_t, Max>>;

        explicit ByteInputStream(ConstByteRange storage);
        ByteInputStream(ConstByteRange storage, const SoftFail&);
        ByteInputStream(ConstByteRange storage, const NoFail&);
    };
}

#endif
