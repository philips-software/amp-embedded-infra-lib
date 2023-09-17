#ifndef PROTOBUF_BUFFERING_STREAM_READER_HPP
#define PROTOBUF_BUFFERING_STREAM_READER_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace services
{
    class BufferingStreamReader
        : public infra::StreamReaderWithRewinding
    {
    public:
        BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::ConstByteRange& data);

        void ConsumeCurrent();
        void StoreRemainder();

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
        infra::BoundedDeque<uint8_t>& buffer;
        infra::ConstByteRange& data;
        std::size_t index = 0;
    };
}

#endif
