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
        void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        ConstByteRange ExtractContiguousRange(std::size_t max) override;
        ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;

    private:
        JsonString string;
        JsonStringIterator position;
    };

    class JsonInputStream
        : public TextInputStream::WithReader<JsonInputStreamReader>
    {
    public:
        explicit JsonInputStream(JsonString storage);
        JsonInputStream(JsonString storage, const SoftFail&);
        JsonInputStream(JsonString storage, const NoFail&);
    };
}

#endif
