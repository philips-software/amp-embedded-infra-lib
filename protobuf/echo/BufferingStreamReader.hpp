#ifndef PROTOBUF_BUFFERING_STREAM_READER_HPP
#define PROTOBUF_BUFFERING_STREAM_READER_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace services
{
    // Usage: Everything that is not read from the inputData is stored into the buffer upon destruction of the BufferingStreamReader
    // Any data already present in the buffer is read first from the reader
    class BufferingStreamReader
        : public infra::StreamReaderWithRewinding
    {
    public:
        BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::ConstByteRange inputData);
        ~BufferingStreamReader();

        // Implementation of StreamReaderWithRewinding
        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        void Rewind(std::size_t marker) override;

    private:
        void StoreRemainder();

    private:
        infra::BoundedDeque<uint8_t>& buffer;
        infra::ConstByteRange inputData;
        std::size_t index = 0;
    };
}

#endif
