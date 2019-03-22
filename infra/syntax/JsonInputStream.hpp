#ifndef INFRA_JSON_INPUT_STREAM_HPP
#define INFRA_JSON_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/syntax/Json.hpp"

namespace infra
{
    class JsonInputStreamReader
        : public StreamReader
    {
    public:
        explicit JsonInputStreamReader(JsonString string);

    public:
        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;

    private:
        JsonString string;
        JsonStringIterator position;
    };

    class JsonInputStream
        : public TextInputStream::WithReader<JsonInputStreamReader>
    {
    public:
        JsonInputStream(JsonString storage);
        JsonInputStream(JsonString storage, const SoftFail&);
        JsonInputStream(JsonString storage, const NoFail&);
    };
}

#endif
