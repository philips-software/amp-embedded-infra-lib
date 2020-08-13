#ifndef INFRA_BOUNDED_VECTOR_INPUT_STREAM_HPP
#define INFRA_BOUNDED_VECTOR_INPUT_STREAM_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/util/BoundedVector.hpp"

namespace infra
{
    class BoundedVectorInputStreamReader
        : public StreamReaderWithRewinding
    {
    public:
        template<std::size_t Max>
            using WithStorage = infra::WithStorage<BoundedVectorInputStreamReader, infra::BoundedVector<uint8_t>::WithMaxSize<Max>>;

        explicit BoundedVectorInputStreamReader(infra::BoundedVector<uint8_t>& container);

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
        infra::BoundedVector<uint8_t>& container;
        std::size_t offset = 0;
    };

    class BoundedVectorInputStream
        : public DataInputStream::WithReader<BoundedVectorInputStreamReader>
    {
    public:
        template<std::size_t Max>
            using WithStorage = infra::WithStorage<DataInputStream::WithReader<BoundedVectorInputStreamReader>, infra::BoundedVector<uint8_t>::WithMaxSize<Max>>;

        BoundedVectorInputStream(infra::BoundedVector<uint8_t>& storage);
        BoundedVectorInputStream(infra::BoundedVector<uint8_t>& storage, const SoftFail&);
        BoundedVectorInputStream(infra::BoundedVector<uint8_t>& storage, const NoFail&);
    };
}

#endif
