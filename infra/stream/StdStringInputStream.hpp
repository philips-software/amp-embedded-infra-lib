#ifndef INFRA_STD_STRING_INPUT_STREAM_HPP
#define INFRA_STD_STRING_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/WithStorage.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace infra
{
    class StdStringInputStreamReader
        : public StreamReader
    {
    public:
        explicit StdStringInputStreamReader(std::string_view string);

    private:
        void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(StreamErrorPolicy& errorPolicy) override;
        ConstByteRange ExtractContiguousRange(std::size_t max) override;
        ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;

    private:
        std::size_t offset = 0;
        std::string_view string;
    };

    class StdStringInputStream
        : public TextInputStream::WithReader<StdStringInputStreamReader>
    {
    public:
        using WithStorage = infra::WithStorage<TextInputStream::WithReader<StdStringInputStreamReader>, std::string>;

        explicit StdStringInputStream(std::string_view storage);
        StdStringInputStream(std::string_view storage, const SoftFail&);
        StdStringInputStream(std::string_view storage, const NoFail&);
    };
}

#endif
