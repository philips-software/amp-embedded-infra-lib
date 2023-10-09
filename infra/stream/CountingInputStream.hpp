#ifndef INFRA_COUNTING_INPUT_STREAM_HPP
#define INFRA_COUNTING_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"

namespace infra
{
    class CountingStreamReader
        : public infra::StreamReader
        , private infra::StreamErrorPolicy
    {
    public:
        explicit CountingStreamReader(infra::StreamReader& reader);

        uint32_t TotalRead() const;

        // Implementation of StreamReader
        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;

        // Implementation of StreamErrorPolicy
        bool Failed() const override;
        void ReportResult(bool ok) override;

    private:
        infra::StreamReader& reader;
        infra::StreamErrorPolicy* otherErrorPolicy = nullptr;
        bool ok = true;
        uint32_t totalRead = 0;
    };

    class CountingStreamReaderWithRewinding
        : public infra::StreamReaderWithRewinding
        , private infra::StreamErrorPolicy
    {
    public:
        explicit CountingStreamReaderWithRewinding(infra::StreamReaderWithRewinding& reader);

        uint32_t TotalRead() const;

        // Implementation of StreamReaderWithRewinding
        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        void Rewind(std::size_t marker) override;

        // Implementation of StreamErrorPolicy
        bool Failed() const override;
        void ReportResult(bool ok) override;

    private:
        infra::StreamReaderWithRewinding& reader;
        infra::StreamErrorPolicy* otherErrorPolicy = nullptr;
        bool ok = true;
        uint32_t totalRead = 0;
    };

    class CountingTextInputStream
        : public TextInputStream::WithReader<CountingStreamReader>
    {
    public:
        using TextInputStream::WithReader<CountingStreamReader>::WithReader;
    };

    class CountingDataInputStream
        : public DataInputStream::WithReader<CountingStreamReader>
    {
    public:
        using DataInputStream::WithReader<CountingStreamReader>::WithReader;
    };
}

#endif
