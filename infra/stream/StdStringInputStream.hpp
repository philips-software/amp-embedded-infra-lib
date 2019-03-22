#ifndef INFRA_STD_STRING_INPUT_STREAM_HPP
#define INFRA_STD_STRING_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/WithStorage.hpp"
#include <cstdint>
#include <string>

namespace infra
{
    class StdStringInputStreamReader
        : public StreamReader
    {
    public:
        explicit StdStringInputStreamReader(const std::string& string);

    private:
        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) override;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) override;
        virtual bool Empty() const override;
        virtual std::size_t Available() const override;

    private:
        uint32_t offset = 0;
        const std::string& string;
    };

    class StdStringInputStream
        : public TextInputStream::WithReader<StdStringInputStreamReader>
    {
    public:
        using WithStorage = infra::WithStorage<TextInputStream::WithReader<StdStringInputStreamReader>, std::string>;

        StdStringInputStream(std::string& storage);
        StdStringInputStream(std::string& storage, const SoftFail&);
        StdStringInputStream(std::string& storage, const NoFail&);
    };
}

#endif
