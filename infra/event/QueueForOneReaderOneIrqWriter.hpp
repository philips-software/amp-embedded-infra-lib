#ifndef INFRA_QUEUE_FOR_ONE_READER_ONE_IRQ_WRITER_HPP
#define INFRA_QUEUE_FOR_ONE_READER_ONE_IRQ_WRITER_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/WithStorage.hpp"
#include <array>
#include <atomic>
#include <cstdint>

namespace infra
{
    class QueueForOneReaderOneIrqWriter
    {
    public:
        template<std::size_t Size>
            using WithStorage = infra::WithStorage<QueueForOneReaderOneIrqWriter, std::array<uint8_t, Size + 1>>;

        QueueForOneReaderOneIrqWriter(const infra::ByteRange& buffer, const infra::Function<void()>& onDataAvailable);

        void AddFromInterrupt(uint8_t element);
        void AddFromInterrupt(infra::ConstByteRange data);

        bool Empty() const;
        bool Full() const;
        uint8_t Get();
        infra::ConstByteRange ContiguousRange(uint32_t offset = 0) const;
        void Consume(uint32_t amount);
        std::size_t Size() const;
        std::size_t EmptySize() const;

        uint8_t operator [] (size_t position) const;

    private:
        bool Full(const uint8_t* begin, const uint8_t* end) const;
        void NotifyDataAvailable();
        void DataAvailable();

    private:
        infra::ByteRange buffer;
        std::atomic<uint8_t*> contentsBegin;
        std::atomic<uint8_t*> contentsEnd;

        infra::Function<void()> onDataAvailable;
        std::atomic_bool notificationScheduled;
    };
}

#endif
