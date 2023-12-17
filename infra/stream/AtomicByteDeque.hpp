#ifndef INFRA_STREAMS_ATOMIC_BYTE_DEQUEUE_HPP
#define INFRA_STREAMS_ATOMIC_BYTE_DEQUEUE_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/WithStorage.hpp"
#include <array>

namespace infra
{
    class AtomicByteDeque
    {
    public:
        template<std::size_t Size>
        using WithStorage = infra::WithStorage<AtomicByteDeque, std::array<uint8_t, Size + 1>>;

        explicit AtomicByteDeque(infra::ByteRange storage);

        void Push(infra::ConstByteRange range);
        void Pop(std::size_t size);

        std::size_t Size() const;
        std::size_t MaxSize() const;
        bool Empty() const;

        infra::ConstByteRange PeekContiguousRange(std::size_t start) const;

    public:
        infra::ByteRange storage;
        std::atomic<uint8_t*> b{ storage.begin() };
        std::atomic<uint8_t*> e{ storage.begin() };
    };

    class AtomicByteDequeReader
        : public infra::StreamReaderWithRewinding
    {
    public:
        explicit AtomicByteDequeReader(AtomicByteDeque& deque);

        void Commit();

        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        void Rewind(std::size_t marker) override;

    private:
        AtomicByteDeque& deque;
        std::size_t offset = 0;
    };

    class AtomicByteDequeWriter
        : public infra::StreamWriter
    {
    public:
        explicit AtomicByteDequeWriter(AtomicByteDeque& deque);

        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        infra::ByteRange SaveState(std::size_t marker) override;
        void RestoreState(infra::ByteRange range) override;
        infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        AtomicByteDeque& deque;
    };
}

#endif
