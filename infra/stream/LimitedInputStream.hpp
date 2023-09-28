#ifndef INFRA_LIMITED_INPUT_STREAM_HPP
#define INFRA_LIMITED_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    class LimitedStreamReader
        : public StreamReader
    {
    public:
        template<class T>
        using WithInput = infra::WithStorage<LimitedStreamReader, T>;

        LimitedStreamReader(StreamReader& input, uint32_t length);
        LimitedStreamReader(const LimitedStreamReader& other);
        ~LimitedStreamReader() = default;

        void ResetLength(uint32_t newLength);

    public:
        void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        ConstByteRange ExtractContiguousRange(std::size_t max) override;
        ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;

    private:
        StreamReader& input;
        uint32_t length;
    };

    class LimitedStreamReaderWithRewinding
        : public StreamReaderWithRewinding
    {
    public:
        template<class T>
        using WithInput = infra::WithStorage<LimitedStreamReaderWithRewinding, T>;

        LimitedStreamReaderWithRewinding(StreamReaderWithRewinding& input, uint32_t length);
        LimitedStreamReaderWithRewinding(const LimitedStreamReaderWithRewinding& other);
        ~LimitedStreamReaderWithRewinding() = default;

        void ResetLength(uint32_t newLength);
        bool LimitReached() const;
        void SwitchInput(StreamReaderWithRewinding& input);

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
        StreamReaderWithRewinding* input;
        uint32_t length;
    };

    class LimitedTextInputStream
        : public TextInputStream::WithReader<LimitedStreamReader>
    {
    public:
        using TextInputStream::WithReader<LimitedStreamReader>::WithReader;
    };

    class LimitedDataInputStream
        : public DataInputStream::WithReader<LimitedStreamReader>
    {
    public:
        using DataInputStream::WithReader<LimitedStreamReader>::WithReader;
    };
}

#endif
