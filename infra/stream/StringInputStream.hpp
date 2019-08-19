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
        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;
        virtual std::size_t ConstructSaveMarker() const override;
        virtual void Rewind(std::size_t marker) override;


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

        StringInputStream(BoundedConstString storage);
        StringInputStream(BoundedConstString storage, const SoftFail&);
        StringInputStream(BoundedConstString storage, const NoFail&);
    };
}

#endif
