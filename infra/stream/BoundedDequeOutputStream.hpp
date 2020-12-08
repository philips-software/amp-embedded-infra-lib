#ifndef INFRA_BOUNDED_DEQUE_OUTPUT_STREAM_HPP
#define INFRA_BOUNDED_DEQUE_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace infra
{
    class BoundedDequeStreamWriter
        : public StreamWriter
    {
    public:
        explicit BoundedDequeStreamWriter(BoundedDeque<uint8_t>& deque);

        void Reset();
        void Reset(BoundedDeque<uint8_t>& newDeque);

    private:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

        virtual std::size_t ConstructSaveMarker() const override;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        virtual infra::ByteRange SaveState(std::size_t marker) override;
        virtual void RestoreState(infra::ByteRange range) override;
        virtual infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        BoundedDeque<uint8_t>* deque;
    };

    class BoundedDequeOutputStream
        : public DataOutputStream::WithWriter<BoundedDequeStreamWriter>
    {
    public:
        template<std::size_t Max>
            using WithStorage = infra::WithStorage<DataOutputStream::WithWriter<BoundedDequeStreamWriter>, BoundedDeque<uint8_t>::WithMaxSize<Max>>;

        explicit BoundedDequeOutputStream(BoundedDeque<uint8_t>& storage);
        BoundedDequeOutputStream(BoundedDeque<uint8_t>& storage, const SoftFail&);
        BoundedDequeOutputStream(BoundedDeque<uint8_t>& storage, const NoFail&);
    };
}

#endif
