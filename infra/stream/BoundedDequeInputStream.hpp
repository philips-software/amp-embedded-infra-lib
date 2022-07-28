#ifndef INFRA_BOUNDED_DEQUE_INPUT_STREAM_HPP
#define INFRA_BOUNDED_DEQUE_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace infra
{
    class BoundedDequeInputStreamReader
        : public StreamReaderWithRewinding
    {
    public:
        template<std::size_t Max>
            using WithStorage = infra::WithStorage<BoundedDequeInputStreamReader, infra::BoundedDeque<uint8_t>::WithMaxSize<Max>>;

        explicit BoundedDequeInputStreamReader(infra::BoundedDeque<uint8_t>& container);

        std::size_t Processed() const;

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
        infra::BoundedDeque<uint8_t>& container;
        std::size_t offset = 0;
    };

    class BoundedDequeInputStream
        : public DataInputStream::WithReader<BoundedDequeInputStreamReader>
    {
    public:
        template<std::size_t Max>
            using WithStorage = infra::WithStorage<DataInputStream::WithReader<BoundedDequeInputStreamReader>, infra::BoundedDeque<uint8_t>::WithMaxSize<Max>>;

        BoundedDequeInputStream(infra::BoundedDeque<uint8_t>& storage);
        BoundedDequeInputStream(infra::BoundedDeque<uint8_t>& storage, const SoftFail&);
        BoundedDequeInputStream(infra::BoundedDeque<uint8_t>& storage, const NoFail&);
    };
}

#endif
