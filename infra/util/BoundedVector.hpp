#ifndef INFRA_BOUNDED_VECTOR_HPP
#define INFRA_BOUNDED_VECTOR_HPP

//  BoundedVector is similar to std::vector, except that it can contain a maximum number of elements

#include "infra/util/MemoryRange.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/StaticStorage.hpp"
#include "infra/util/WithStorage.hpp"
#include <algorithm>
#include <array>
#include <iterator>

namespace infra
{
    template<class T>
    class BoundedVector
    {
    public:
        template<std::size_t Max>
        using WithMaxSize = infra::WithStorage<BoundedVector<T>, std::array<StaticStorage<T>, Max>>;

        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using difference_type = typename std::iterator_traits<iterator>::difference_type;
        using size_type = std::size_t;

    public:
        BoundedVector(const BoundedVector& other) = delete;
        explicit BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage);
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, size_type n, const value_type& value = value_type());
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, infra::MemoryRange<typename std::remove_const<T>::type> initData);
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, infra::MemoryRange<const T> initData);
        template<class InputIterator>
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, InputIterator first, InputIterator last);
        template<class U>
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, std::initializer_list<U> initializerList);
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, const BoundedVector& other);
        BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, BoundedVector&& other) noexcept;
        ~BoundedVector();

        BoundedVector& operator=(const BoundedVector& other);
        BoundedVector& operator=(BoundedVector&& other) noexcept;
        void AssignFromStorage(const BoundedVector& other);
        void AssignFromStorage(BoundedVector&& other) noexcept;

    public:
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

        reverse_iterator rbegin();
        const_reverse_iterator rbegin() const;
        reverse_iterator rend();
        const_reverse_iterator rend() const;

        const_iterator cbegin() const;
        const_iterator cend() const;
        const_reverse_iterator crbegin() const;
        const_reverse_iterator crend() const;

    public:
        size_type size() const;
        size_type max_size() const;
        void resize(size_type n, const value_type& value = value_type());
        bool empty() const;
        bool full() const;
        infra::MemoryRange<T> range();
        infra::MemoryRange<const T> range() const;

    public:
        value_type& operator[](size_type position);
        const value_type& operator[](size_type position) const;
        value_type& front();
        const value_type& front() const;
        value_type& back();
        const value_type& back() const;
        value_type* data();
        const value_type* data() const;

    public:
        void assign(infra::MemoryRange<typename std::remove_const<T>::type> initData);
        void assign(infra::MemoryRange<const T> initData);
        template<class InputIterator>
        void assign(InputIterator first, InputIterator last);
        void assign(size_type n, const value_type& value);

        void push_back(const value_type& value);
        void push_back(value_type&& value);
        void pop_back();

        iterator insert(iterator position, const value_type& value);
        void insert(iterator position, size_type n, const value_type& value);
        template<class InputIterator>
        void insert(iterator position, InputIterator first, InputIterator last);

        iterator erase(iterator position);
        iterator erase(iterator first, iterator last);

        void swap(BoundedVector& other) noexcept;

        void clear();

        template<class... Args>
        iterator emplace(const_iterator position, Args&&... args);
        template<class... Args>
        void emplace_back(Args&&... args);

    public:
        bool operator==(const BoundedVector& other) const;
        bool operator!=(const BoundedVector& other) const;
        bool operator<(const BoundedVector& other) const;
        bool operator<=(const BoundedVector& other) const;
        bool operator>(const BoundedVector& other) const;
        bool operator>=(const BoundedVector& other) const;

    private:
        void move_up(const_iterator position, size_type n);

    private:
        infra::MemoryRange<infra::StaticStorage<T>> storage;
        size_type numAllocated = 0;
    };

    template<class T>
    void swap(BoundedVector<T>& x, BoundedVector<T>& y) noexcept;

    template<class T>
    typename BoundedVector<T>::size_type erase(BoundedVector<T>& c, const T& value);

    template<class T, class Pred>
    typename BoundedVector<T>::size_type erase_if(BoundedVector<T>& c, Pred pred);

    template<class T>
    MemoryRange<T> MakeRange(infra::BoundedVector<T>& container);
    template<class T>
    MemoryRange<const T> MakeRange(const infra::BoundedVector<T>& container);

    //// Implementation ////

    template<class T>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage)
        : storage(storage)
    {}

    template<class T>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, size_type n, const value_type& value)
        : storage(storage)
    {
        resize(n, value);
    }

    template<class T>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, infra::MemoryRange<typename std::remove_const<T>::type> initData)
        : storage(storage)
    {
        assign(initData);
    }

    template<class T>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, infra::MemoryRange<const T> initData)
        : storage(storage)
    {
        assign(initData);
    }

    template<class T>
    template<class InputIterator>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, InputIterator first, InputIterator last)
        : storage(storage)
    {
        assign(first, last);
    }

    template<class T>
    template<class U>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, std::initializer_list<U> initializerList)
        : storage(storage)
    {
        assign(initializerList.begin(), initializerList.end());
    }

    template<class T>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, const BoundedVector& other)
        : storage(storage)
    {
        assign(other.begin(), other.end());
    }

    template<class T>
    BoundedVector<T>::BoundedVector(infra::MemoryRange<infra::StaticStorage<T>> storage, BoundedVector&& other) noexcept
        : storage(storage)
        , numAllocated(other.numAllocated)
    {
        for (size_type i = 0; i != size(); ++i)
            storage[i].Construct(std::move(other[i]));
    }

    template<class T>
    BoundedVector<T>& BoundedVector<T>::operator=(const BoundedVector& other)
    {
        if (this != &other)
            assign(other.begin(), other.end());

        return *this;
    }

    template<class T>
    BoundedVector<T>& BoundedVector<T>::operator=(BoundedVector<T>&& other) noexcept
    {
        clear();

        numAllocated = other.size();

        for (size_type i = 0; i != size(); ++i)
            storage[i].Construct(std::move(other[i]));

        return *this;
    }

    template<class T>
    void BoundedVector<T>::AssignFromStorage(const BoundedVector& other)
    {
        *this = other;
    }

    template<class T>
    void BoundedVector<T>::AssignFromStorage(BoundedVector&& other) noexcept
    {
        *this = std::move(other);
    }

    template<class T>
    BoundedVector<T>::~BoundedVector()
    {
        clear();
    }

    template<class T>
    typename BoundedVector<T>::iterator BoundedVector<T>::begin()
    {
        if (!storage.empty())
            return &*storage[0];
        else
            return nullptr;
    }

    template<class T>
    typename BoundedVector<T>::const_iterator BoundedVector<T>::begin() const
    {
        if (!storage.empty())
            return &*storage[0];
        else
            return nullptr;
    }

    template<class T>
    typename BoundedVector<T>::iterator BoundedVector<T>::end()
    {
        return begin() + size();
    }

    template<class T>
    typename BoundedVector<T>::const_iterator BoundedVector<T>::end() const
    {
        return begin() + size();
    }

    template<class T>
    typename BoundedVector<T>::reverse_iterator BoundedVector<T>::rbegin()
    {
        return reverse_iterator(end());
    }

    template<class T>
    typename BoundedVector<T>::const_reverse_iterator BoundedVector<T>::rbegin() const
    {
        return const_reverse_iterator(end());
    }

    template<class T>
    typename BoundedVector<T>::reverse_iterator BoundedVector<T>::rend()
    {
        return reverse_iterator(begin());
    }

    template<class T>
    typename BoundedVector<T>::const_reverse_iterator BoundedVector<T>::rend() const
    {
        return const_reverse_iterator(begin());
    }

    template<class T>
    typename BoundedVector<T>::const_iterator BoundedVector<T>::cbegin() const
    {
        return begin();
    }

    template<class T>
    typename BoundedVector<T>::const_iterator BoundedVector<T>::cend() const
    {
        return end();
    }

    template<class T>
    typename BoundedVector<T>::const_reverse_iterator BoundedVector<T>::crbegin() const
    {
        return rbegin();
    }

    template<class T>
    typename BoundedVector<T>::const_reverse_iterator BoundedVector<T>::crend() const
    {
        return rend();
    }

    template<class T>
    typename BoundedVector<T>::size_type BoundedVector<T>::size() const
    {
        return numAllocated;
    }

    template<class T>
    typename BoundedVector<T>::size_type BoundedVector<T>::max_size() const
    {
        return storage.size();
    }

    template<class T>
    void BoundedVector<T>::resize(size_type n, const value_type& value)
    {
        while (n < size())
            pop_back();

        while (n > size())
            push_back(value);
    }

    template<class T>
    bool BoundedVector<T>::empty() const
    {
        return numAllocated == 0;
    }

    template<class T>
    bool BoundedVector<T>::full() const
    {
        return numAllocated == max_size();
    }

    template<class T>
    infra::MemoryRange<T> BoundedVector<T>::range()
    {
        return infra::MemoryRange<T>(begin(), end());
    }

    template<class T>
    infra::MemoryRange<const T> BoundedVector<T>::range() const
    {
        return infra::MemoryRange<const T>(begin(), end());
    }

    template<class T>
    typename BoundedVector<T>::value_type& BoundedVector<T>::operator[](size_type position)
    {
        really_assert(position < size());
        return *storage[position];
    }

    template<class T>
    const typename BoundedVector<T>::value_type& BoundedVector<T>::operator[](size_type position) const
    {
        really_assert(position < size());
        return *storage[position];
    }

    template<class T>
    typename BoundedVector<T>::value_type& BoundedVector<T>::front()
    {
        really_assert(!empty());
        return *storage[0];
    }

    template<class T>
    const typename BoundedVector<T>::value_type& BoundedVector<T>::front() const
    {
        really_assert(!empty());
        return *storage[0];
    }

    template<class T>
    typename BoundedVector<T>::value_type& BoundedVector<T>::back()
    {
        really_assert(!empty());
        return *storage[size() - 1];
    }

    template<class T>
    const typename BoundedVector<T>::value_type& BoundedVector<T>::back() const
    {
        really_assert(!empty());
        return *storage[size() - 1];
    }

    template<class T>
    typename BoundedVector<T>::value_type* BoundedVector<T>::data()
    {
        return begin();
    }

    template<class T>
    const typename BoundedVector<T>::value_type* BoundedVector<T>::data() const
    {
        return begin();
    }

    template<class T>
    void BoundedVector<T>::assign(infra::MemoryRange<typename std::remove_const<T>::type> initData)
    {
        assign(initData.begin(), initData.end());
    }

    template<class T>
    void BoundedVector<T>::assign(infra::MemoryRange<const T> initData)
    {
        assign(initData.begin(), initData.end());
    }

    template<class T>
    template<class InputIterator>
    void BoundedVector<T>::assign(InputIterator first, InputIterator last)
    {
        clear();

        while (first != last)
        {
            emplace_back(*first);
            ++first;
        }
    }

    template<class T>
    void BoundedVector<T>::assign(size_type n, const value_type& value)
    {
        clear();

        for (size_type i = 0; i != n; ++i)
            emplace_back(value);
    }

    template<class T>
    void BoundedVector<T>::push_back(const value_type& value)
    {
        really_assert(!full());
        storage[numAllocated].Construct(value);
        ++numAllocated;
    }

    template<class T>
    void BoundedVector<T>::push_back(value_type&& value)
    {
        really_assert(!full());
        storage[numAllocated].Construct(std::move(value));
        ++numAllocated;
    }

    template<class T>
    void BoundedVector<T>::pop_back()
    {
        really_assert(!empty());
        storage[numAllocated - 1].Destruct();
        --numAllocated;
    }

    template<class T>
    typename BoundedVector<T>::iterator BoundedVector<T>::insert(iterator position, const value_type& value)
    {
        insert(position, size_type(1), value);
        return position;
    }

    template<class T>
    void BoundedVector<T>::insert(iterator position, size_type n, const value_type& value)
    {
        really_assert(size() + n <= max_size());

        auto initialized_size = std::min(std::min(position - begin() + n, size()), n);

        move_up(position, n);
        std::fill_n(position, initialized_size, value);
        std::uninitialized_fill_n(position + initialized_size, n - initialized_size, value);
    }

    template<class T>
    template<class RandomAccessIterator>
    void BoundedVector<T>::insert(iterator position, RandomAccessIterator first, RandomAccessIterator last)
    {
        size_type n = last - first;
        really_assert(size() + n <= max_size());

        auto initialized_size = std::min(std::min(position - begin() + n, size()), n);
        auto last_initialized = first + initialized_size;

        move_up(position, n);
        std::copy(first, last_initialized, position);
        std::uninitialized_copy(last_initialized, last, position + initialized_size);
    }

    template<class T>
    typename BoundedVector<T>::iterator BoundedVector<T>::erase(iterator position)
    {
        erase(position, position + 1);
        return position;
    }

    template<class T>
    typename BoundedVector<T>::iterator BoundedVector<T>::erase(iterator first, iterator last)
    {
        iterator result = first;

        while (last != end())
        {
            *first = std::move(*last);
            ++first;
            ++last;
        }

        while (first != last)
        {
            storage[first - begin()].Destruct();
            ++first;
            --numAllocated;
        }

        return result;
    }

    template<class T>
    void BoundedVector<T>::swap(BoundedVector& other) noexcept
    {
        using std::swap;

        assert(size() <= other.max_size() && other.size() < max_size());

        for (size_type i = 0; i < size() && i < other.size(); ++i)
        {
            swap(*storage[i], *other.storage[i]);
        }

        for (size_type i = size(); i < other.size(); ++i)
        {
            storage[i].Construct(std::move(*other.storage[i]));
            other.storage[i].Destruct();
        }

        for (size_type i = other.size(); i < size(); ++i)
        {
            other.storage[i].Construct(std::move(*storage[i]));
            storage[i].Destruct();
        }

        std::swap(numAllocated, other.numAllocated);
    }

    template<class T>
    void BoundedVector<T>::clear()
    {
        while (!empty())
            pop_back();
    }

    template<class T>
    template<class... Args>
    typename BoundedVector<T>::iterator BoundedVector<T>::emplace(const_iterator position, Args&&... args)
    {
        really_assert(size() != max_size());
        move_up(position, 1);
        storage[position - begin()].Construct(std::forward<Args>(args)...);
        return begin() + (position - begin());
    }

    template<class T>
    template<class... Args>
    void BoundedVector<T>::emplace_back(Args&&... args)
    {
        really_assert(size() != max_size());
        storage[numAllocated].Construct(std::forward<Args>(args)...);
        ++numAllocated;
    }

    template<class T>
    bool BoundedVector<T>::operator==(const BoundedVector<T>& other) const
    {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }

    template<class T>
    bool BoundedVector<T>::operator!=(const BoundedVector<T>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool BoundedVector<T>::operator<(const BoundedVector<T>& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    template<class T>
    bool BoundedVector<T>::operator<=(const BoundedVector<T>& other) const
    {
        return !(other < *this);
    }

    template<class T>
    bool BoundedVector<T>::operator>(const BoundedVector<T>& other) const
    {
        return other < *this;
    }

    template<class T>
    bool BoundedVector<T>::operator>=(const BoundedVector<T>& other) const
    {
        return !(*this < other);
    }

    template<class T>
    void BoundedVector<T>::move_up(const_iterator position, size_type n)
    {
        iterator copy_position = end();
        while (copy_position != position && copy_position + n != end())
        {
            storage[copy_position - begin() + n - 1].Construct(std::move(*(copy_position - 1)));
            --copy_position;
        }

        while (copy_position != position)
        {
            *storage[copy_position - begin() + n - 1] = std::move(*(copy_position - 1));
            --copy_position;
        }

        numAllocated += n;
    }

    template<class T>
    void swap(BoundedVector<T>& x, BoundedVector<T>& y) noexcept
    {
        x.swap(y);
    }

    template<class T>
    typename BoundedVector<T>::size_type erase(BoundedVector<T>& c, const T& value)
    {
        return erase_if(c, [&value](const auto& v)
            {
                return v == value;
            });
    }

    template<class T, class Pred>
    typename BoundedVector<T>::size_type erase_if(BoundedVector<T>& c, Pred pred)
    {
        auto in = c.begin();
        auto out = c.begin();
        while (in != c.end())
        {
            if (pred(*in))
                ++in;
            else
            {
                if (in != out)
                    *out = std::move(*in);

                ++in;
                ++out;
            }
        }

        auto result = c.end() - out;
        c.erase(out, c.end());
        return result;
    }

    template<class T>
    MemoryRange<T> MakeRange(infra::BoundedVector<T>& container)
    {
        return MemoryRange<T>(container.begin(), container.end());
    }

    template<class T>
    MemoryRange<const T> MakeRange(const infra::BoundedVector<T>& container)
    {
        return MemoryRange<const T>(container.begin(), container.end());
    }
}

#endif
