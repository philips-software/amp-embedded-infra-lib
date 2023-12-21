#ifndef INFRA_BOUNDED_DEQUE_HPP
#define INFRA_BOUNDED_DEQUE_HPP

//  BoundedDeque is similar to std::deque, except that it can contain a maximum number of elements.

#include "infra/util/MemoryRange.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/StaticStorage.hpp"
#include "infra/util/WithStorage.hpp"
#include <algorithm>
#include <array>
#include <iterator>

namespace infra
{
    namespace detail
    {
        template<class DequeType, class T>
        class BoundedDequeIterator;
    }

    template<class T>
    class BoundedDeque
    {
    public:
        template<std::size_t Max>
        using WithMaxSize = infra::WithStorage<BoundedDeque<T>, std::array<StaticStorage<T>, Max>>;

        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = detail::BoundedDequeIterator<BoundedDeque<T>, T>;
        using const_iterator = detail::BoundedDequeIterator<const BoundedDeque<T>, const T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using difference_type = typename std::iterator_traits<iterator>::difference_type;
        using size_type = std::size_t;

    public:
        BoundedDeque() = default;
        explicit BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage);
        BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, size_type n, const value_type& value = value_type());
        template<class InputIterator>
        BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, InputIterator first, InputIterator last);
        BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, const BoundedDeque& other);
        BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, BoundedDeque&& other) noexcept;
        BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, std::initializer_list<T> initializerList);
        BoundedDeque& operator=(const BoundedDeque& other);
        BoundedDeque& operator=(BoundedDeque&& other) noexcept;
        BoundedDeque& operator=(std::initializer_list<T> initializerList);
        void AssignFromStorage(const BoundedDeque& other);
        void AssignFromStorage(BoundedDeque&& other) noexcept;
        ~BoundedDeque();

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
        infra::MemoryRange<T> contiguous_range(const_iterator from);
        infra::MemoryRange<const T> contiguous_range(const_iterator from) const;
        infra::MemoryRange<T> contiguous_range_at_end(const_iterator to);
        infra::MemoryRange<const T> contiguous_range_at_end(const_iterator to) const;

    public:
        value_type& operator[](size_type position);
        const value_type& operator[](size_type position) const;
        value_type& front();
        const value_type& front() const;
        value_type& back();
        const value_type& back() const;

    public:
        template<class InputIterator>
        void assign(InputIterator first, const InputIterator& last);
        void assign(size_type n, const value_type& value);
        void assign(std::initializer_list<T> initializerList);

        void push_front(const value_type& value);
        void push_front(value_type&& value);
        void pop_front();
        void push_back(const value_type& value);
        void push_back(value_type&& value);
        void pop_back();

        iterator insert(const const_iterator& position, const value_type& value);
        iterator insert(const const_iterator& position, size_type n, const value_type& value);
        template<class InputIterator>
        iterator insert(const const_iterator& position, InputIterator first, const InputIterator& last);
        iterator insert(const const_iterator& position, T&& val);
        iterator insert(const const_iterator& position, std::initializer_list<T> initializerList);

        iterator erase(const const_iterator& position);
        iterator erase(const const_iterator& first, const const_iterator& last);

        void swap(BoundedDeque& other) noexcept;

        void clear();

        template<class... Args>
        iterator emplace(const const_iterator& position, Args&&... args);
        template<class... Args>
        void emplace_front(Args&&... args);
        template<class... Args>
        void emplace_back(Args&&... args);

    public:
        bool operator==(const BoundedDeque& other) const;
        bool operator!=(const BoundedDeque& other) const;
        bool operator<(const BoundedDeque& other) const;
        bool operator<=(const BoundedDeque& other) const;
        bool operator>(const BoundedDeque& other) const;
        bool operator>=(const BoundedDeque& other) const;

    private:
        size_type index(size_type i) const;
        void move_up(size_type position, size_type n);

    private:
        infra::MemoryRange<StaticStorage<T>> storage;
        size_type numAllocated = 0;
        size_type start = 0;
    };

    template<class T>
    void swap(BoundedDeque<T>& x, BoundedDeque<T>& y) noexcept;

    namespace detail
    {
        template<class DequeType, class T>
        class BoundedDequeIterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            BoundedDequeIterator() = delete;
            BoundedDequeIterator(DequeType* deque, std::size_t offset);
            template<class DequeType2, class T2>
            BoundedDequeIterator(const BoundedDequeIterator<DequeType2, T2>& other);
            template<class DequeType2, class T2>
            BoundedDequeIterator& operator=(const BoundedDequeIterator<DequeType2, T2>& other);

            T& operator*() const;
            T* operator->() const;
            T& operator[](std::ptrdiff_t offset) const;
            BoundedDequeIterator& operator++();
            BoundedDequeIterator operator++(int);
            BoundedDequeIterator& operator--();
            BoundedDequeIterator operator--(int);

            BoundedDequeIterator& operator+=(std::ptrdiff_t offset);
            BoundedDequeIterator operator+(std::ptrdiff_t offset) const;
            BoundedDequeIterator& operator-=(std::ptrdiff_t offset);
            BoundedDequeIterator operator-(std::ptrdiff_t offset) const;
            std::ptrdiff_t operator-(BoundedDequeIterator other) const;

            template<class DequeType2, class T2>
            bool operator==(const BoundedDequeIterator<DequeType2, T2>& other) const;
            template<class DequeType2, class T2>
            bool operator!=(const BoundedDequeIterator<DequeType2, T2>& other) const;
            template<class DequeType2, class T2>
            bool operator<(const BoundedDequeIterator<DequeType2, T2>& other) const;
            template<class DequeType2, class T2>
            bool operator>(const BoundedDequeIterator<DequeType2, T2>& other) const;
            template<class DequeType2, class T2>
            bool operator<=(const BoundedDequeIterator<DequeType2, T2>& other) const;
            template<class DequeType2, class T2>
            bool operator>=(const BoundedDequeIterator<DequeType2, T2>& other) const;

        private:
            template<class, class>
            friend class BoundedDequeIterator;
            friend DequeType;

            std::size_t index;
            DequeType* deque;
        };
    }

    //// Implementation ////

    template<class T>
    BoundedDeque<T>::BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage)
        : storage(storage)
    {}

    template<class T>
    BoundedDeque<T>::BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, size_type n, const value_type& value)
        : storage(storage)
    {
        resize(n, value);
    }

    template<class T>
    template<class InputIterator>
    BoundedDeque<T>::BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, InputIterator first, InputIterator last)
        : storage(storage)
    {
        assign(first, last);
    }

    template<class T>
    BoundedDeque<T>::BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, const BoundedDeque& other)
        : storage(storage)
    {
        assign(other.begin(), other.end());
    }

    template<class T>
    BoundedDeque<T>::BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, BoundedDeque&& other) noexcept
        : storage(storage)
        , numAllocated(other.numAllocated)
    {
        for (size_type i = 0; i != size(); ++i)
            storage[i].Construct(std::move(other[i]));
    }

    template<class T>
    BoundedDeque<T>::BoundedDeque(infra::MemoryRange<infra::StaticStorage<T>> storage, std::initializer_list<T> initializerList)
        : storage(storage)
    {
        assign(initializerList.begin(), initializerList.end());
    }

    template<class T>
    BoundedDeque<T>& BoundedDeque<T>::operator=(const BoundedDeque& other)
    {
        if (this != &other)
            assign(other.begin(), other.end());

        return *this;
    }

    template<class T>
    BoundedDeque<T>& BoundedDeque<T>::operator=(BoundedDeque<T>&& other) noexcept
    {
        clear();

        numAllocated = other.numAllocated;

        for (size_type i = 0; i != size(); ++i)
            storage[i].Construct(std::move(other[i]));

        return *this;
    }

    template<class T>
    BoundedDeque<T>& BoundedDeque<T>::operator=(std::initializer_list<T> initializerList)
    {
        assign(initializerList.begin(), initializerList.end());

        return *this;
    }

    template<class T>
    void BoundedDeque<T>::AssignFromStorage(const BoundedDeque& other)
    {
        *this = other;
    }

    template<class T>
    void BoundedDeque<T>::AssignFromStorage(BoundedDeque&& other) noexcept
    {
        *this = std::move(other);
    }

    template<class T>
    BoundedDeque<T>::~BoundedDeque()
    {
        clear();
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::begin()
    {
        return iterator(this, 0);
    }

    template<class T>
    typename BoundedDeque<T>::const_iterator BoundedDeque<T>::begin() const
    {
        return const_iterator(this, 0);
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::end()
    {
        return iterator(this, numAllocated);
    }

    template<class T>
    typename BoundedDeque<T>::const_iterator BoundedDeque<T>::end() const
    {
        return const_iterator(this, numAllocated);
    }

    template<class T>
    typename BoundedDeque<T>::reverse_iterator BoundedDeque<T>::rbegin()
    {
        return reverse_iterator(end());
    }

    template<class T>
    typename BoundedDeque<T>::const_reverse_iterator BoundedDeque<T>::rbegin() const
    {
        return const_reverse_iterator(end());
    }

    template<class T>
    typename BoundedDeque<T>::reverse_iterator BoundedDeque<T>::rend()
    {
        return reverse_iterator(begin());
    }

    template<class T>
    typename BoundedDeque<T>::const_reverse_iterator BoundedDeque<T>::rend() const
    {
        return const_reverse_iterator(begin());
    }

    template<class T>
    typename BoundedDeque<T>::const_iterator BoundedDeque<T>::cbegin() const
    {
        return begin();
    }

    template<class T>
    typename BoundedDeque<T>::const_iterator BoundedDeque<T>::cend() const
    {
        return end();
    }

    template<class T>
    typename BoundedDeque<T>::const_reverse_iterator BoundedDeque<T>::crbegin() const
    {
        return rbegin();
    }

    template<class T>
    typename BoundedDeque<T>::const_reverse_iterator BoundedDeque<T>::crend() const
    {
        return rend();
    }

    template<class T>
    typename BoundedDeque<T>::size_type BoundedDeque<T>::size() const
    {
        return numAllocated;
    }

    template<class T>
    typename BoundedDeque<T>::size_type BoundedDeque<T>::max_size() const
    {
        return storage.size();
    }

    template<class T>
    void BoundedDeque<T>::resize(size_type n, const value_type& value)
    {
        while (n < size())
            pop_back();

        while (n > size())
            push_back(value);
    }

    template<class T>
    bool BoundedDeque<T>::empty() const
    {
        return numAllocated == 0;
    }

    template<class T>
    bool BoundedDeque<T>::full() const
    {
        return numAllocated == max_size();
    }

    template<class T>
    infra::MemoryRange<T> BoundedDeque<T>::contiguous_range(const_iterator from)
    {
        if (!empty())
        {
            std::size_t i = index(from.index);
            return infra::MemoryRange<T>(&*storage[i], std::min(&*storage[i] + numAllocated - from.index, &**storage.begin() + storage.size()));
        }
        else
            return infra::MemoryRange<T>();
    }

    template<class T>
    infra::MemoryRange<const T> BoundedDeque<T>::contiguous_range(const_iterator from) const
    {
        if (!empty())
        {
            std::size_t i = index(from.index);
            return infra::MemoryRange<const T>(&*storage[i], std::min(&*storage[i] + numAllocated - from.index, &**storage.begin() + storage.size()));
        }
        else
            return infra::MemoryRange<const T>();
    }

    template<class T>
    infra::MemoryRange<T> BoundedDeque<T>::contiguous_range_at_end(const_iterator to)
    {
        if (!empty())
        {
            std::size_t i = index(to.index);
            if (i == 0)
                i = storage.size();
            return infra::MemoryRange<T>(std::max((&**storage.begin()) + i - to.index, &**storage.begin()), (&**storage.begin()) + i);
        }
        else
            return infra::MemoryRange<T>();
    }

    template<class T>
    infra::MemoryRange<const T> BoundedDeque<T>::contiguous_range_at_end(const_iterator to) const
    {
        if (!empty())
        {
            std::size_t i = index(to.index);
            if (i == 0)
                i = storage.size();
            return infra::MemoryRange<T>(std::max((&**storage.begin()) + i - to.index, &**storage.begin()), (&**storage.begin()) + i);
        }
        else
            return infra::MemoryRange<T>();
    }

    template<class T>
    typename BoundedDeque<T>::value_type& BoundedDeque<T>::operator[](size_type position)
    {
        really_assert(position >= 0 && position < size());
        return *storage[index(position)];
    }

    template<class T>
    const typename BoundedDeque<T>::value_type& BoundedDeque<T>::operator[](size_type position) const
    {
        really_assert(position >= 0 && position < size());
        return *storage[index(position)];
    }

    template<class T>
    typename BoundedDeque<T>::value_type& BoundedDeque<T>::front()
    {
        really_assert(!empty());
        return *storage[start];
    }

    template<class T>
    const typename BoundedDeque<T>::value_type& BoundedDeque<T>::front() const
    {
        really_assert(!empty());
        return *storage[start];
    }

    template<class T>
    typename BoundedDeque<T>::value_type& BoundedDeque<T>::back()
    {
        really_assert(!empty());
        return *storage[index(numAllocated - 1)];
    }

    template<class T>
    const typename BoundedDeque<T>::value_type& BoundedDeque<T>::back() const
    {
        really_assert(!empty());
        return *storage[index(numAllocated - 1)];
    }

    template<class T>
    template<class InputIterator>
    void BoundedDeque<T>::assign(InputIterator first, const InputIterator& last)
    {
        clear();

        for (; first != last; ++first)
            push_back(*first);
    }

    template<class T>
    void BoundedDeque<T>::assign(size_type n, const value_type& value)
    {
        clear();

        for (size_type i = 0; i != n; ++i)
            push_back(value);
    }

    template<class T>
    void BoundedDeque<T>::assign(std::initializer_list<T> initializerList)
    {
        assign(initializerList.begin(), initializerList.end());
    }

    template<class T>
    void BoundedDeque<T>::push_front(const value_type& value)
    {
        really_assert(!full());
        start = index(max_size() - 1);
        storage[start].Construct(value);
        ++numAllocated;
    }

    template<class T>
    void BoundedDeque<T>::push_front(value_type&& value)
    {
        really_assert(!full());
        start = index(max_size() - 1);
        storage[start].Construct(std::move(value));
        ++numAllocated;
    }

    template<class T>
    void BoundedDeque<T>::pop_front()
    {
        really_assert(!empty());
        storage[start].Destruct();
        start = index(1);
        --numAllocated;
    }

    template<class T>
    void BoundedDeque<T>::push_back(const value_type& value)
    {
        really_assert(!full());
        storage[index(numAllocated)].Construct(value);
        ++numAllocated;
    }

    template<class T>
    void BoundedDeque<T>::push_back(value_type&& value)
    {
        really_assert(!full());
        storage[index(numAllocated)].Construct(std::move(value));
        ++numAllocated;
    }

    template<class T>
    void BoundedDeque<T>::pop_back()
    {
        really_assert(!empty());
        storage[index(numAllocated - 1)].Destruct();
        --numAllocated;
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::insert(const const_iterator& position, const value_type& value)
    {
        return insert(position, size_type(1), value);
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::insert(const const_iterator& position, size_type n, const value_type& value)
    {
        really_assert(size() + n <= max_size());

        size_type element_index = position - begin();

        if (element_index == 0 && numAllocated != 0)
        {
            start = index(max_size() - n);
            numAllocated += n;

            for (size_type i = 0, e = element_index; i != n; ++i, ++e)
                storage[index(e)].Construct(value);
        }
        else
        {
            move_up(element_index, n);

            for (size_type i = 0, e = element_index; i != n; ++i, ++e)
                *storage[index(e)] = value;
        }

        return iterator(this, element_index);
    }

    template<class T>
    template<class RandomAccessIterator>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::insert(const const_iterator& position, RandomAccessIterator first, const RandomAccessIterator& last)
    {
        size_type n = last - first;
        really_assert(size() + n <= max_size());

        size_type element_index = position - begin();

        if (element_index == 0 && numAllocated != 0)
        {
            start = index(max_size() - n);
            numAllocated += n;

            for (; first != last; ++element_index, ++first)
                storage[index(element_index)].Construct(*first);
        }
        else
        {
            move_up(element_index, n);

            for (; first != last; ++element_index, ++first)
                *storage[index(element_index)] = *first;
        }

        return iterator(this, position - begin());
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::insert(const const_iterator& position, T&& val)
    {
        return emplace(position, std::move(val));
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::insert(const const_iterator& position, std::initializer_list<T> initializerList)
    {
        return insert(position, initializerList.begin(), initializerList.end());
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::erase(const const_iterator& position)
    {
        return erase(position, position + 1);
    }

    template<class T>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::erase(const const_iterator& first, const const_iterator& last)
    {
        size_type first_index = first - begin();
        size_type last_index = last - begin();

        if (first_index == 0)
        {
            for (; last_index != 0; --numAllocated, ++start, --last_index)
                storage[index(0)].Destruct();
        }
        else
        {
            for (; last_index != numAllocated; ++first_index, ++last_index)
                *storage[index(first_index)] = std::move(*storage[index(last_index)]);

            for (; first_index != last_index; ++first_index, --numAllocated)
                storage[index(first_index)].Destruct();
        }

        return iterator(this, first - begin());
    }

    template<class T>
    void BoundedDeque<T>::swap(BoundedDeque& other) noexcept
    {
        using std::swap;

        for (size_type i = 0; i < size() && i < other.size(); ++i)
            swap(*storage[index(i)], *other.storage[other.index(i)]);

        for (size_type i = size(); i < other.size(); ++i)
        {
            storage[index(i)].Construct(std::move(*other.storage[other.index(i)]));
            other.storage[other.index(i)].Destruct();
        }

        for (size_type i = other.size(); i < size(); ++i)
        {
            other.storage[other.index(i)].Construct(std::move(*storage[index(i)]));
            storage[index(i)].Destruct();
        }

        std::swap(numAllocated, other.numAllocated);
    }

    template<class T>
    void BoundedDeque<T>::clear()
    {
        if (std::is_trivially_destructible<T>::value)
            numAllocated = 0;
        else
            while (!empty())
                pop_back();

        start = 0;
    }

    template<class T>
    template<class... Args>
    typename BoundedDeque<T>::iterator BoundedDeque<T>::emplace(const const_iterator& position, Args&&... args)
    {
        really_assert(size() != max_size());
        size_type element_index = position - begin();
        move_up(element_index, 1);
        storage[index(element_index)].Construct(std::forward<Args>(args)...);

        return iterator(this, element_index);
    }

    template<class T>
    template<class... Args>
    void BoundedDeque<T>::emplace_front(Args&&... args)
    {
        really_assert(size() != max_size());
        if (start != 0)
            --start;
        else
            start = max_size() - 1;
        storage[index(0)].Construct(std::forward<Args>(args)...);
        ++numAllocated;
    }

    template<class T>
    template<class... Args>
    void BoundedDeque<T>::emplace_back(Args&&... args)
    {
        really_assert(size() != max_size());

        storage[index(numAllocated)].Construct(std::forward<Args>(args)...);
        ++numAllocated;
    }

    template<class T>
    bool BoundedDeque<T>::operator==(const BoundedDeque<T>& other) const
    {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }

    template<class T>
    bool BoundedDeque<T>::operator!=(const BoundedDeque<T>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool BoundedDeque<T>::operator<(const BoundedDeque<T>& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    template<class T>
    bool BoundedDeque<T>::operator<=(const BoundedDeque<T>& other) const
    {
        return !(other < *this);
    }

    template<class T>
    bool BoundedDeque<T>::operator>(const BoundedDeque<T>& other) const
    {
        return other < *this;
    }

    template<class T>
    bool BoundedDeque<T>::operator>=(const BoundedDeque<T>& other) const
    {
        return !(*this < other);
    }

    template<class T>
    typename BoundedDeque<T>::size_type BoundedDeque<T>::index(size_type elementIndex) const
    {
        size_type index = start + elementIndex;
        if (index < max_size())
            return index;
        else
            return index - max_size();
    }

    template<class T>
    void BoundedDeque<T>::move_up(size_type position, size_type n)
    {
        size_type number_of_copies = numAllocated - position;
        size_type copy_position = numAllocated - 1 + n;

        for (; number_of_copies != 0 && copy_position >= numAllocated; --number_of_copies, --copy_position)
            storage[index(copy_position)].Construct(std::move(*storage[index(copy_position - n)]));

        for (; number_of_copies != 0; --number_of_copies, --copy_position)
            *storage[index(copy_position)] = std::move(*storage[index(copy_position - n)]);

        numAllocated += n;
    }

    template<class T>
    void swap(BoundedDeque<T>& x, BoundedDeque<T>& y) noexcept
    {
        x.swap(y);
    }

    namespace detail
    {
        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T>::BoundedDequeIterator(DequeType* deque, std::size_t offset)
            : index(offset)
            , deque(deque)
        {}

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        BoundedDequeIterator<DequeType, T>::BoundedDequeIterator(const BoundedDequeIterator<DequeType2, T2>& other)
            : index(other.index)
            , deque(other.deque)
        {}

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        BoundedDequeIterator<DequeType, T>& BoundedDequeIterator<DequeType, T>::operator=(const BoundedDequeIterator<DequeType2, T2>& other)
        {
            deque = other.deque;
            index = other.index;

            return *this;
        }

        template<class DequeType, class T>
        T& BoundedDequeIterator<DequeType, T>::operator*() const
        {
            return (*deque)[index];
        }

        template<class DequeType, class T>
        T* BoundedDequeIterator<DequeType, T>::operator->() const
        {
            return &(*deque)[index];
        }

        template<class DequeType, class T>
        T& BoundedDequeIterator<DequeType, T>::operator[](std::ptrdiff_t offset) const
        {
            return (*deque)[index + offset];
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T>& BoundedDequeIterator<DequeType, T>::operator++()
        {
            ++index;
            return *this;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T> BoundedDequeIterator<DequeType, T>::operator++(int)
        {
            BoundedDequeIterator copy(*this);
            ++index;
            return copy;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T>& BoundedDequeIterator<DequeType, T>::operator--()
        {
            --index;
            return *this;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T> BoundedDequeIterator<DequeType, T>::operator--(int)
        {
            BoundedDequeIterator copy(*this);
            --index;
            return copy;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T>& BoundedDequeIterator<DequeType, T>::operator+=(std::ptrdiff_t offset)
        {
            index += offset;
            return *this;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T> BoundedDequeIterator<DequeType, T>::operator+(std::ptrdiff_t offset) const
        {
            BoundedDequeIterator copy(*this);
            copy += offset;
            return copy;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T>& BoundedDequeIterator<DequeType, T>::operator-=(std::ptrdiff_t offset)
        {
            index -= offset;
            return *this;
        }

        template<class DequeType, class T>
        BoundedDequeIterator<DequeType, T> BoundedDequeIterator<DequeType, T>::operator-(std::ptrdiff_t offset) const
        {
            BoundedDequeIterator copy(*this);
            copy -= offset;
            return copy;
        }

        template<class DequeType, class T>
        std::ptrdiff_t BoundedDequeIterator<DequeType, T>::operator-(BoundedDequeIterator other) const
        {
            return index - other.index;
        }

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        bool BoundedDequeIterator<DequeType, T>::operator==(const BoundedDequeIterator<DequeType2, T2>& other) const
        {
            really_assert(deque == other.deque);
            return index == other.index;
        }

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        bool BoundedDequeIterator<DequeType, T>::operator!=(const BoundedDequeIterator<DequeType2, T2>& other) const
        {
            return !(*this == other);
        }

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        bool BoundedDequeIterator<DequeType, T>::operator<(const BoundedDequeIterator<DequeType2, T2>& other) const
        {
            really_assert(deque == other.deque);
            return index < other.index;
        }

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        bool BoundedDequeIterator<DequeType, T>::operator>(const BoundedDequeIterator<DequeType2, T2>& other) const
        {
            return other < *this;
        }

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        bool BoundedDequeIterator<DequeType, T>::operator<=(const BoundedDequeIterator<DequeType2, T2>& other) const
        {
            return !(other < *this);
        }

        template<class DequeType, class T>
        template<class DequeType2, class T2>
        bool BoundedDequeIterator<DequeType, T>::operator>=(const BoundedDequeIterator<DequeType2, T2>& other) const
        {
            return !(*this < other);
        }
    }
}

#endif
