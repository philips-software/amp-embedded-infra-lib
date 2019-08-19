#ifndef INFRA_MEMORY_RANGE_HPP
#define INFRA_MEMORY_RANGE_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <type_traits>
#include <vector>

namespace infra
{
    template<class T>
    class MemoryRange
    {
    public:
        MemoryRange();
        MemoryRange(T* begin, T* end);
        template<class U>                                                           //TICS !INT#001
            MemoryRange(const MemoryRange<U>& other);
        template<class T2, std::size_t N>                                           //TICS !INT#001
            constexpr MemoryRange(const std::array<T2, N>& array);
        template<class T2, std::size_t N>                                           //TICS !INT#001
            MemoryRange(std::array<T2, N>& array);
        template<class T2>                                                          //TICS !INT#001
            MemoryRange(const std::vector<T2>& vector);
        template<class T2>                                                          //TICS !INT#001
            MemoryRange(std::vector<T2>& vector);

        bool empty() const;
        std::size_t size() const;

        // MemoryRange does not own its elements, so accessing an element is a const operation that returns a non-const element
        T* begin() const;
        const T* cbegin() const;
        T* end() const;
        const T* cend() const;

        T& operator[](std::size_t index) const;
        template<class U>
            bool operator==(const MemoryRange<U>& rhs) const;
        template<class U>
            bool operator!=(const MemoryRange<U>& rhs) const;

        T& front() const;
        T& back() const;

        bool contains(const T* element) const;
        bool contains_or_end(const T* element) const;

        void clear();
        void pop_front(std::size_t num = 1);
        void pop_back(std::size_t num = 1);
        void shrink_from_front_to(std::size_t newSize);
        void shrink_from_back_to(std::size_t newSize);

    private:
        T* beginElement;
        T* endElement;
    };

    template<class T>
        MemoryRange<T> MakeRange(T* b, T* e);
    template<class T, std::size_t N>
        MemoryRange<T> MakeRange(std::array<T, N>& container);
    template<class T, std::size_t N>
        MemoryRange<const T> MakeRange(const std::array<T, N>& container);
    template<class T>
        std::vector<typename std::remove_const<T>::type> MakeVector(MemoryRange<T> range);
    template<class T>
        MemoryRange<T> MakeRange(std::vector<T>& range);
    template<class T>
        MemoryRange<const T> MakeRange(const std::vector<T>& range);
    template<class T>
        MemoryRange<const T> MakeConst(infra::MemoryRange<T> range);

    template<class T, class U>
        MemoryRange<T> ReinterpretCastMemoryRange(MemoryRange<U> memoryRange);
    template<class T>
        MemoryRange<T> ConstCastMemoryRange(MemoryRange<const T> memoryRange);
    template<class T>
        MemoryRange<T> VolatileCastMemoryRange(MemoryRange<volatile T> memoryRange);

    template<class T>
        void Copy(MemoryRange<const typename std::remove_const<T>::type> from, MemoryRange<T> to);
    template<class T, class U>
        bool ContentsEqual(MemoryRange<T> x, MemoryRange<U> y);

    template<class T>
        MemoryRange<T> IntersectingRange(MemoryRange<T> x, MemoryRange<T> y);
    template<class T>
        MemoryRange<T> Head(MemoryRange<T> range, std::size_t size);
    template<class T>
        MemoryRange<T> Tail(MemoryRange<T> range, std::size_t size);
    template<class T>
        MemoryRange<T> DiscardHead(MemoryRange<T> range, std::size_t size);
    template<class T>
        MemoryRange<T> DiscardTail(MemoryRange<T> range, std::size_t size);

    template<class T, class U>
        T Convert(MemoryRange<U> range);

    template<class T, class U, std::size_t N>
        bool operator==(MemoryRange<T> x, const std::array<U, N>& y);
    template<class T, class U, std::size_t N>
        bool operator==(const std::array<U, N>& x, MemoryRange<T> y);
    template<class T, class U, std::size_t N>
        bool operator!=(MemoryRange<T> x, const std::array<U, N>& y);
    template<class T, class U, std::size_t N>
        bool operator!=(const std::array<U, N>& x, MemoryRange<T> y);

    template<class T, class U>
        bool operator==(MemoryRange<T> x, const std::vector<U>& y);
    template<class T, class U>
        bool operator==(const std::vector<U>& x, MemoryRange<T> y);
    template<class T, class U>
        bool operator!=(MemoryRange<T> x, const std::vector<U>& y);
    template<class T, class U>
        bool operator!=(const std::vector<U>& x, MemoryRange<T> y);

    ////    Implementation    ////

    template<class T>
    MemoryRange<T>::MemoryRange()
        : beginElement(nullptr)
        , endElement(nullptr)
    {}

    template<class T>
    MemoryRange<T>::MemoryRange(T* begin, T* end)
        : beginElement(begin)
        , endElement(end)
    {}

    template<class T>
    template<class U>
    MemoryRange<T>::MemoryRange(const MemoryRange<U>& other)
        : beginElement(other.begin())
        , endElement(other.end())
    {}

    template<class T>
    template<class T2, std::size_t N>
    constexpr MemoryRange<T>::MemoryRange(const std::array<T2, N>& array)
        : beginElement(N != 0 ? &array.front() : nullptr)
        , endElement(N != 0 ? &array.front() + array.size() : nullptr)
    {}

    template<class T>
    template<class T2, std::size_t N>
    MemoryRange<T>::MemoryRange(std::array<T2, N>& array)
        : beginElement(array.data())
        , endElement(array.data() + array.size())
    {}

    template<class T>
    template<class T2>
    MemoryRange<T>::MemoryRange(const std::vector<T2>& vector)
        : beginElement(vector.data())
        , endElement(vector.data() + vector.size())
    {}

    template<class T>
    template<class T2>
    MemoryRange<T>::MemoryRange(std::vector<T2>& vector)
        : beginElement(vector.data())
        , endElement(vector.data() + vector.size())
    {}

    template<class T>
    bool MemoryRange<T>::empty() const
    {
        return beginElement == endElement;
    }

    template<class T>
    std::size_t MemoryRange<T>::size() const
    {
        return endElement - beginElement;
    }

    template<class T>
    T* MemoryRange<T>::begin() const
    {
        return beginElement;
    }

    template<class T>
    const T* MemoryRange<T>::cbegin() const
    {
        return beginElement;
    }

    template<class T>
    T* MemoryRange<T>::end() const
    {
        return endElement;
    }

    template<class T>
    const T* MemoryRange<T>::cend() const
    {
        return endElement;
    }

    template<class T>
    T& MemoryRange<T>::operator[](std::size_t index) const
    {
        assert(index < size());
        return *(begin() + index);
    }

    template<class T>
    template<class U>
    bool MemoryRange<T>::operator==(const MemoryRange<U>& rhs) const
    {
        return begin() == rhs.begin() && end() == rhs.end();
    }

    template<class T>
    template<class U>
    bool MemoryRange<T>::operator!=(const MemoryRange<U>& rhs) const
    {
        return !(*this == rhs);
    }

    template<class T>
    T& MemoryRange<T>::front() const
    {
        return *beginElement;
    }

    template<class T>
    T& MemoryRange<T>::back() const
    {
        return *(endElement - 1);
    }

    template<class T>
    bool MemoryRange<T>::contains(const T* element) const
    {
        return element >= beginElement && element < endElement;
    }

    template<class T>
    bool MemoryRange<T>::contains_or_end(const T* element) const
    {
        return element >= beginElement && element <= endElement;
    }

    template<class T>
    void MemoryRange<T>::clear()
    {
        beginElement = nullptr;
        endElement = nullptr;
    }

    template<class T>
    void MemoryRange<T>::pop_front(std::size_t num)
    {
        assert(num <= size());
        beginElement += num;
    }

    template<class T>
    void MemoryRange<T>::pop_back(std::size_t num)
    {
        assert(num <= size());
        endElement -= num;
    }

    template<class T>
    void MemoryRange<T>::shrink_from_front_to(std::size_t newSize)
    {
        if (newSize < size())
            beginElement = endElement - newSize;
    }

    template<class T>
    void MemoryRange<T>::shrink_from_back_to(std::size_t newSize)
    {
        if (newSize < size())
            endElement = beginElement + newSize;
    }

    template<class T>
    MemoryRange<T> MakeRange(T* b, T* e)
    {
        return MemoryRange<T>(b, e);
    }

    template<class T, std::size_t N>
    MemoryRange<T> MakeRange(T(&data)[N])
    {
        return MemoryRange<T>(&data[0], &data[0] + N);
    }

    template<class T, std::size_t N>
    MemoryRange<const T> MakeRange(const T(&data)[N])
    {
        return MemoryRange<const T>(&data[0], &data[0] + N);
    }

    template<class T, std::size_t N>
    MemoryRange<T> MakeRange(std::array<T, N>& container)
    {
        return MemoryRange<T>(container.data(), container.data() + container.size());
    }

    template<class T, std::size_t N>
    MemoryRange<const T> MakeRange(const std::array<T, N>& container)
    {
        return MemoryRange<const T>(container.data(), container.data() + container.size());
    }

    template<class T>
    std::vector<typename std::remove_const<T>::type> MakeVector(MemoryRange<T> range)
    {
        return std::vector<typename std::remove_const<T>::type>(range.begin(), range.end());
    }

    template<class T>
    MemoryRange<T> MakeRange(std::vector<T>& range)
    {
        return MemoryRange<T>(range);
    }

    template<class T>
    MemoryRange<const T> MakeRange(const std::vector<T>& range)
    {
        return MemoryRange<const T>(range);
    }

    template<class T>
    MemoryRange<const T> MakeConst(infra::MemoryRange<T> range)
    {
        return MemoryRange<const T>(range);
    }

    template<class T, class U>
    MemoryRange<T> ReinterpretCastMemoryRange(MemoryRange<U> memoryRange)
    {
        return MemoryRange<T>(reinterpret_cast<T*>(memoryRange.begin()), reinterpret_cast<T*>(memoryRange.end()));
    }

    template<class T>
    MemoryRange<T> ConstCastMemoryRange(MemoryRange<const T> memoryRange)
    {
        return MemoryRange<T>(const_cast<T*>(memoryRange.begin()), const_cast<T*>(memoryRange.end()));
    }

    template<class T>
    MemoryRange<T> VolatileCastMemoryRange(MemoryRange<volatile T> memoryRange)
    {
        return MemoryRange<T>(const_cast<T*>(memoryRange.begin()), const_cast<T*>(memoryRange.end()));
    }

    template<class T>
    void Copy(MemoryRange<const typename std::remove_const<T>::type> from, MemoryRange<T> to)
    {
        assert(from.size() == to.size());
        std::copy(from.begin(), from.begin() + from.size(), to.begin());
    }

    template<class T, class U>
    bool ContentsEqual(MemoryRange<T> x, MemoryRange<U> y)
    {
        return x.size() == y.size()
            && std::equal(x.begin(), x.end(), y.begin());
    }

    template<class T>
    MemoryRange<T> IntersectingRange(MemoryRange<T> x, MemoryRange<T> y)
    {
        if (x.begin() > y.end() || x.end() < y.begin())
            return MemoryRange<T>();

        return MemoryRange<T>(std::max(x.begin(), y.begin()), std::min(x.end(), y.end()));
    }

    template<class T>
    MemoryRange<T> Head(MemoryRange<T> range, std::size_t size)
    {
        range.shrink_from_back_to(size);
        return range;
    }

    template<class T>
    MemoryRange<T> Tail(MemoryRange<T> range, std::size_t size)
    {
        range.shrink_from_front_to(size);
        return range;
    }

    template<class T>
    MemoryRange<T> DiscardHead(MemoryRange<T> range, std::size_t size)
    {
        if (range.size() <= size)
            return MemoryRange<T>();

        range.pop_front(size);
        return range;
    }

    template<class T>
    MemoryRange<T> DiscardTail(MemoryRange<T> range, std::size_t size)
    {
        if (range.size() <= size)
            return MemoryRange<T>();

        range.pop_back(size);
        return range;
    }

    template<class T, class U>
    T Convert(MemoryRange<U> range)
    {
        T result{};
        Copy(range, infra::ReinterpretCastMemoryRange<typename std::remove_const<U>::type>(infra::MemoryRange<T>(&result, &result + 1)));
        return result;
    }

    template<class T, class U, std::size_t N>
    bool operator==(MemoryRange<T> x, const std::array<U, N>& y)
    {
        return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin());
    }

    template<class T, class U, std::size_t N>
    bool operator==(const std::array<U, N>& x, MemoryRange<T> y)
    {
        return y == x;
    }

    template<class T, class U, std::size_t N>
    bool operator!=(MemoryRange<T> x, const std::array<U, N>& y)
    {
        return !(x == y);
    }

    template<class T, class U, std::size_t N>
    bool operator!=(const std::array<U, N>& x, MemoryRange<T> y)
    {
        return !(x == y);
    }

    template<class T, class U>
    bool operator==(MemoryRange<T> x, const std::vector<U>& y)
    {
        return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin());
    }

    template<class T, class U>
    bool operator==(const std::vector<U>& x, MemoryRange<T> y)
    {
        return y == x;
    }

    template<class T, class U>
    bool operator!=(MemoryRange<T> x, const std::vector<U>& y)
    {
        return !(x == y);
    }

    template<class T, class U>
    bool operator!=(const std::vector<U>& x, MemoryRange<T> y)
    {
        return !(x == y);
    }
}

#endif
