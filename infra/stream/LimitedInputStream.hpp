#ifndef INFRA_LIMITED_INPUT_STREAM_HPP
#define INFRA_LIMITED_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"

namespace infra
{
    class LimitedStreamReader
        : public StreamReader
    {
    public:
        LimitedStreamReader(StreamReader& input, uint32_t length);
        LimitedStreamReader(const LimitedStreamReader& other);

    public:
        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;

    private:
        StreamReader& input;
        uint32_t length;
    };

    class LimitedStreamReaderWithRewinding
        : public StreamReaderWithRewinding
    {
    public:
        LimitedStreamReaderWithRewinding(StreamReaderWithRewinding& input, uint32_t length);
        LimitedStreamReaderWithRewinding(const LimitedStreamReaderWithRewinding& other);

    public:
        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;
        virtual std::size_t ConstructSaveMarker() const override;
        virtual void Rewind(std::size_t marker) override;

    private:
        StreamReaderWithRewinding& input;
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
