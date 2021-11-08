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
        CountingStreamReader(infra::StreamReader& reader);

        uint32_t TotalRead() const;

        // Implementation of StreamReader
        virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;

        // Implementation of StreamErrorPolicy
        virtual bool Failed() const override;
        virtual void ReportResult(bool ok) override;

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
        CountingStreamReaderWithRewinding(infra::StreamReaderWithRewinding& reader);

        uint32_t TotalRead() const;

        // Implementation of StreamReaderWithRewinding
        virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;
        virtual std::size_t ConstructSaveMarker() const override;
        virtual void Rewind(std::size_t marker) override;

        // Implementation of StreamErrorPolicy
        virtual bool Failed() const override;
        virtual void ReportResult(bool ok) override;

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
