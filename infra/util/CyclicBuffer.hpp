#ifndef INFRA_CYCLIC_BUFFER_HPP
#define INFRA_CYCLIC_BUFFER_HPP

//  CyclicBuffer transforms a given chunk of memory into a cyclic buffer. Data can be pushed into this buffer,
//  and popped out again. With ContiguousRange(), the largest block starting at the start can be obtained. This
//  is useful when feeding large blocks to e.g. DMA.
//  In practise, the typedef CyclicByteBuffer will most often be used.

#include "infra/util/ByteRange.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    template<class T>
    class CyclicBuffer
    {
    public:
        template<std::size_t StorageSize>
        using WithStorage = infra::WithStorage<CyclicBuffer<T>, std::array<T, StorageSize>>;

        explicit CyclicBuffer(MemoryRange<T> aStorage);

        bool Empty() const;
        bool Full() const;
        std::size_t Size() const;
        std::size_t Available() const;

        void Push(T data);
        void Push(MemoryRange<const T> data);

        void Pop(std::size_t numToPop);

        MemoryRange<T> ContiguousRange(size_t offset = 0) const;

    private:
        MemoryRange<T> storage;
        T* front;
        T* back;
        bool isFull{ false };
    };

    using CyclicByteBuffer = CyclicBuffer<uint8_t>;

    ////    Implementation    ////

    template<class T>
    CyclicBuffer<T>::CyclicBuffer(MemoryRange<T> aStorage)
        : storage(aStorage)
        , front(storage.begin())
        , back(storage.begin())
    {}

    template<class T>
    bool CyclicBuffer<T>::Empty() const
    {
        return front == back && !isFull;
    }

    template<class T>
    bool CyclicBuffer<T>::Full() const
    {
        return isFull;
    }

    template<class T>
    std::size_t CyclicBuffer<T>::Size() const
    {
        if (isFull)
            return storage.size();
        else if (back < front)
            return (storage.end() - front) + (back - storage.begin());
        else
            return back - front;
    }

    template<class T>
    std::size_t CyclicBuffer<T>::Available() const
    {
        return storage.size() - Size();
    }

    template<class T>
    void CyclicBuffer<T>::Push(T data)
    {
        assert(!Full());

        *back = data;
        ++back;

        if (back == storage.end())
            back = storage.begin();

        if (front == back)
            isFull = true;
    }

    template<class T>
    void CyclicBuffer<T>::Push(MemoryRange<const T> data)
    {
        assert(data.size() <= storage.size() - Size());
        std::size_t size = std::min(data.size(), static_cast<std::size_t>(storage.end() - back));
        back = std::copy(data.begin(), data.begin() + size, back);

        if (back == storage.end())
            back = storage.begin();

        if (size < data.size())
            back = std::copy(data.begin() + size, data.end(), back);

        if (!data.empty() && front == back)
            isFull = true;
    }

    template<class T>
    void CyclicBuffer<T>::Pop(std::size_t numToPop)
    {
        if (numToPop != 0)
            isFull = false;

        if (numToPop >= static_cast<std::size_t>(storage.end() - front))
        {
            numToPop -= storage.end() - front;
            front = storage.begin();
        }

        front += numToPop;

        if (Empty())
        {
            front = storage.begin();
            back = storage.begin();
        }
    }

    template<class T>
    MemoryRange<T> CyclicBuffer<T>::ContiguousRange(size_t offset) const
    {
        assert(offset == 0 || offset < Size());

        if (Empty())
            return MemoryRange<T>();

        auto offsetFront = front + offset;
        if (offsetFront >= storage.end())
            offsetFront = storage.begin() + (offsetFront - storage.end());

        if (offsetFront >= back)
            return MemoryRange<T>(offsetFront, storage.end());
        else
            return MemoryRange<T>(offsetFront, back);
    }
}

#endif
