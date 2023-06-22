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
        void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        ConstByteRange ExtractContiguousRange(std::size_t max) override;
        ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;

    private:
        uint32_t offset = 0;
        const std::string& string;
    };

    class StdStringInputStream
        : public TextInputStream::WithReader<StdStringInputStreamReader>
    {
    public:
        using WithStorage = infra::WithStorage<TextInputStream::WithReader<StdStringInputStreamReader>, std::string>;

        explicit StdStringInputStream(const std::string& storage);
        StdStringInputStream(const std::string& storage, const SoftFail&);
        StdStringInputStream(const std::string& storage, const NoFail&);
    };
}

#endif
