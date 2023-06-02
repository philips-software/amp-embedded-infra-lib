#ifndef INFRA_STRING_INPUT_STREAM_HPP
#define INFRA_STRING_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/BoundedString.hpp"

namespace infra
{
    class StringInputStreamReader
        : public StreamReaderWithRewinding
    {
    public:
        explicit StringInputStreamReader(BoundedConstString string);

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
        uint32_t offset = 0;
        BoundedConstString string;
    };

    class StringInputStream
        : public TextInputStream::WithReader<StringInputStreamReader>
    {
    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<TextInputStream::WithReader<StringInputStreamReader>, infra::BoundedString::WithStorage<Max>>;

        explicit StringInputStream(BoundedConstString storage);
        StringInputStream(BoundedConstString storage, const SoftFail&);
        StringInputStream(BoundedConstString storage, const NoFail&);
    };
}

#endif
