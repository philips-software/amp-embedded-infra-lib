#ifndef INFRA_QUEUE_FOR_ONE_READER_ONE_IRQ_WRITER_HPP
#define INFRA_QUEUE_FOR_ONE_READER_ONE_IRQ_WRITER_HPP

#include "infra/util/Function.hpp"
#include "infra/util/WithStorage.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <cassert>
#include "infra/util/BoundedVector.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace infra
{
    template<class T>
    class QueueForOneReaderOneIrqWriter
    {
    public:
        template<std::size_t Size>
            using WithStorage = infra::WithStorage<QueueForOneReaderOneIrqWriter<T>, std::array<T, Size+1>>;

        QueueForOneReaderOneIrqWriter(const infra::MemoryRange<T> buffer, const infra::Function<void()>& onDataAvailable);

        void AddFromInterrupt(T element);
        void AddFromInterrupt(infra::MemoryRange<const T> data);

        bool Empty() const;
        bool Full() const;
        T Get();
        infra::MemoryRange<const T> ContiguousRange(uint32_t offset = 0) const;
        void Consume(uint32_t amount);
        std::size_t Size() const;
        std::size_t EmptySize() const;

        T operator [] (size_t position) const;

    private:
        bool Full(const T* begin, const T* end) const;
        void NotifyDataAvailable();
        void DataAvailable();

    private:
        const infra::MemoryRange<T> buffer;
        std::atomic<T*> contentsBegin;
        std::atomic<T*> contentsEnd;

        infra::Function<void()> onDataAvailable;
        std::atomic_bool notificationScheduled;
    };

    //// Implementation ////

    template<class T>
    QueueForOneReaderOneIrqWriter<T>::QueueForOneReaderOneIrqWriter(const infra::MemoryRange<T> buffer, const infra::Function<void()>& onDataAvailable)
        : buffer(buffer)
        , contentsBegin(buffer.begin())
        , contentsEnd(buffer.begin())
        , onDataAvailable(onDataAvailable)
    {
        static_assert(std::is_trivial<T>::value, "Trivial type required");
        notificationScheduled = false;
    }

    template<class T>
    void QueueForOneReaderOneIrqWriter<T>::AddFromInterrupt(T element)
    {
        assert(!Full());
        *contentsEnd = element;

        if (contentsEnd == buffer.end() - 1)
            contentsEnd = buffer.begin();
        else
            ++contentsEnd;

        NotifyDataAvailable();
    }

    template<class T>
    void QueueForOneReaderOneIrqWriter<T>::AddFromInterrupt(infra::MemoryRange<const T> data)
    {
        std::size_t copySize = std::min<std::size_t>(data.size(), buffer.end() - contentsEnd);
        T* begin = contentsBegin.load();
        if (!(begin <= contentsEnd.load() || begin > contentsEnd.load() + copySize))
            return;

        std::copy(data.begin(), data.begin() + copySize, contentsEnd.load());

        if (contentsEnd == buffer.end() - copySize)
            contentsEnd = buffer.begin();
        else
            contentsEnd += copySize;

        data.pop_front(copySize);

        assert(begin <= contentsEnd.load() || begin > contentsEnd.load() + data.size());
        std::copy(data.begin(), data.end(), contentsEnd.load());
        contentsEnd += data.size();

        NotifyDataAvailable();
    }

    template<class T>
    bool QueueForOneReaderOneIrqWriter<T>::Empty() const
    {
        return contentsBegin.load() == contentsEnd.load();
    }

    template<class T>
    bool QueueForOneReaderOneIrqWriter<T>::Full() const
    {
        const T* begin = contentsBegin.load();
        const T* end = contentsEnd.load();

        return Full(begin, end);
    }

    template<class T>
    bool QueueForOneReaderOneIrqWriter<T>::Full(const T* begin, const T* end) const
    {
        return (end == buffer.end() - 1 || begin == end + 1)
            && (end != buffer.end() - 1 || begin == buffer.begin());
    }

    template <class T>
    T QueueForOneReaderOneIrqWriter<T>::Get()
    {
        assert(!Empty());
        T result = *contentsBegin;

        if (contentsBegin == buffer.end() - 1)
            contentsBegin = buffer.begin();
        else
            ++contentsBegin;

        return result;
    }

    template<class T>
    T QueueForOneReaderOneIrqWriter<T>::operator[](size_t position) const
    {
        size_t size = Size();
        assert(size > 0);
        assert(position < size);

        T* begin = contentsBegin;
        if (begin + position >= buffer.end())
        {
            position -= buffer.end() - begin;
            begin = buffer.begin();
        }

        begin += position;

        return *begin;
    }

    template<class T>
    infra::MemoryRange<const T> QueueForOneReaderOneIrqWriter<T>::ContiguousRange(uint32_t offset) const
    {
        const T* end = contentsEnd.load();
        T* begin = contentsBegin;

        if (begin + offset >= buffer.end())
        {
            offset -= buffer.end() - begin;
            begin = buffer.begin();
        }

        begin += offset;

        if (end < begin)
            return infra::MemoryRange<const T>(begin, buffer.end());
        else
            return infra::MemoryRange<const T>(begin, end);
    }

    template<class T>
    void QueueForOneReaderOneIrqWriter<T>::Consume(uint32_t amount)
    {
        if (contentsBegin + amount >= buffer.end())
        {
            amount -= buffer.end() - contentsBegin;
            contentsBegin = buffer.begin();
        }

        contentsBegin += amount;
    }

    template<class T>
    std::size_t QueueForOneReaderOneIrqWriter<T>::Size() const
    {
        const T* begin = contentsBegin.load();
        const T* end = contentsEnd.load();

        if (Full(begin, end))
            return EmptySize();
        else if (end >= begin)
            return end - begin;

        return (buffer.end() - begin) + (end - buffer.begin());
    }

    template<class T>
    std::size_t QueueForOneReaderOneIrqWriter<T>::EmptySize() const
    {
        return buffer.size() - 1;
    }

    template<class T>
    void QueueForOneReaderOneIrqWriter<T>::NotifyDataAvailable()
    {
        if (!Empty() && !notificationScheduled.exchange(true))
            infra::EventDispatcher::Instance().Schedule([this]() { DataAvailable(); });
    }

    template<class T>
    void QueueForOneReaderOneIrqWriter<T>::DataAvailable()
    {
        notificationScheduled = false;
        onDataAvailable();
    }
}

#endif
