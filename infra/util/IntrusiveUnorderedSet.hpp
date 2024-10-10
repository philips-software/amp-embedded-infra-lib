#ifndef INFRA_INTRUSIVE_UNORDERED_SET_HPP
#define INFRA_INTRUSIVE_UNORDERED_SET_HPP

#include "infra/util/IntrusiveList.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/WithStorage.hpp"
#include <array>
#include <cstdint>
#include <functional>
#include <utility>

namespace infra
{
    template<class T>
    class IntrusiveUnorderedSet
    {
    public:
        using NodeType = typename IntrusiveList<T>::NodeType;

        using value_type = typename IntrusiveList<T>::value_type;
        using reference = typename IntrusiveList<T>::reference;
        using const_reference = typename IntrusiveList<T>::const_reference;
        using pointer = typename IntrusiveList<T>::pointer;
        using const_pointer = typename IntrusiveList<T>::const_pointer;
        using iterator = typename IntrusiveList<T>::iterator;
        using const_iterator = typename IntrusiveList<T>::const_iterator;
        using reverse_iterator = typename IntrusiveList<T>::reverse_iterator;
        using const_reverse_iterator = typename IntrusiveList<T>::const_reverse_iterator;
        using difference_type = typename IntrusiveList<T>::difference_type;
        using size_type = typename IntrusiveList<T>::size_type;

    public:
        template<std::size_t B>
        using WithBuckets = infra::WithStorage<IntrusiveUnorderedSet<T>, std::array<std::pair<iterator, iterator>, B>>;

    public:
        explicit IntrusiveUnorderedSet(infra::MemoryRange<std::pair<iterator, iterator>> buckets);
        template<class InputIterator>
        IntrusiveUnorderedSet(infra::MemoryRange<std::pair<iterator, iterator>> buckets, InputIterator first, InputIterator last);
        IntrusiveUnorderedSet(const IntrusiveUnorderedSet& other) = delete;
        IntrusiveUnorderedSet(infra::MemoryRange<std::pair<iterator, iterator>> buckets, IntrusiveUnorderedSet&& other) noexcept;
        IntrusiveUnorderedSet& operator=(const IntrusiveUnorderedSet& other) = delete;
        IntrusiveUnorderedSet& operator=(IntrusiveUnorderedSet&& other) noexcept;
        void AssignFromStorage(IntrusiveUnorderedSet&& other) noexcept;

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
        bool empty() const;
        size_type size() const;
        bool has_element(const_reference value) const;
        bool contains(const_reference value) const;

    public:
        void insert(reference value);

        template<class... Args>
        std::pair<iterator, bool> emplace(Args&&... args);

        void erase(reference value);

        template<class InputIterator>
        void assign(InputIterator first, InputIterator last);

        void clear();

        void swap(IntrusiveUnorderedSet& other) noexcept;

    public:
        bool operator==(const IntrusiveUnorderedSet& other) const;
        bool operator!=(const IntrusiveUnorderedSet& other) const;

    private:
        void InitializeBuckets();
        uint32_t HashToBucket(const_reference value) const;

    private:
        infra::IntrusiveList<T> values;
        infra::MemoryRange<std::pair<iterator, iterator>> buckets;
    };

    template<class T>
    void swap(IntrusiveUnorderedSet<T>& x, IntrusiveUnorderedSet<T>& y) noexcept;

    ////    Implementation    ////

    template<class T>
    IntrusiveUnorderedSet<T>::IntrusiveUnorderedSet(infra::MemoryRange<std::pair<iterator, iterator>> buckets)
        : buckets(buckets)
    {
        InitializeBuckets();
    }

    template<class T>
    template<class InputIterator>
    IntrusiveUnorderedSet<T>::IntrusiveUnorderedSet(infra::MemoryRange<std::pair<iterator, iterator>> buckets, InputIterator first, InputIterator last)
        : buckets(buckets)
    {
        assign(first, last);
    }

    template<class T>
    IntrusiveUnorderedSet<T>::IntrusiveUnorderedSet(infra::MemoryRange<std::pair<iterator, iterator>> buckets, IntrusiveUnorderedSet&& other) noexcept
        : buckets(buckets)
    {
        *this = std::move(other);
    }

    template<class T>
    IntrusiveUnorderedSet<T>& IntrusiveUnorderedSet<T>::operator=(IntrusiveUnorderedSet&& other) noexcept
    {
        values.clear();
        InitializeBuckets();

        while (!other.values.empty())
        {
            auto& value = other.values.front();
            other.values.erase(value);
            insert(value);
        }

        other.InitializeBuckets();

        return *this;
    }

    template<class T>
    void IntrusiveUnorderedSet<T>::AssignFromStorage(IntrusiveUnorderedSet&& other) noexcept
    {
        *this = std::move(other);
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::iterator IntrusiveUnorderedSet<T>::begin()
    {
        return values.begin();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_iterator IntrusiveUnorderedSet<T>::begin() const
    {
        return values.begin();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::iterator IntrusiveUnorderedSet<T>::end()
    {
        return values.end();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_iterator IntrusiveUnorderedSet<T>::end() const
    {
        return values.end();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::reverse_iterator IntrusiveUnorderedSet<T>::rbegin()
    {
        return values.rbegin();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_reverse_iterator IntrusiveUnorderedSet<T>::rbegin() const
    {
        return values.rbegin();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::reverse_iterator IntrusiveUnorderedSet<T>::rend()
    {
        return values.rend();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_reverse_iterator IntrusiveUnorderedSet<T>::rend() const
    {
        return values.rend();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_iterator IntrusiveUnorderedSet<T>::cbegin() const
    {
        return values.cbegin();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_iterator IntrusiveUnorderedSet<T>::cend() const
    {
        return values.cend();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_reverse_iterator IntrusiveUnorderedSet<T>::crbegin() const
    {
        return values.crbegin();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::const_reverse_iterator IntrusiveUnorderedSet<T>::crend() const
    {
        return values.crend();
    }

    template<class T>
    bool IntrusiveUnorderedSet<T>::empty() const
    {
        return values.empty();
    }

    template<class T>
    typename IntrusiveUnorderedSet<T>::size_type IntrusiveUnorderedSet<T>::size() const
    {
        return values.size();
    }

    template<class T>
    bool IntrusiveUnorderedSet<T>::has_element(const_reference value) const
    {
        auto bucket = buckets[HashToBucket(value)];

        for (auto v = bucket.first; v != bucket.second; ++v)
            if (&*v == &value)
                return true;

        return false;
    }

    template<class T>
    bool IntrusiveUnorderedSet<T>::contains(const_reference value) const
    {
        auto bucket = buckets[HashToBucket(value)];

        for (auto v = bucket.first; v != bucket.second; ++v)
            if (*v == value)
                return true;

        return false;
    }

    template<class T>
    void IntrusiveUnorderedSet<T>::insert(reference value)
    {
        auto& bucket = buckets[HashToBucket(value)];

        values.insert(bucket.first, value);
        bucket.first = std::prev(bucket.second);
    }

    template<class T>
    void IntrusiveUnorderedSet<T>::erase(reference value)
    {
        auto& bucket = buckets[HashToBucket(value)];

        if (&*bucket.first == &value)
            ++bucket.first;
        else if (&*std::prev(bucket.second) == &value)
            --bucket.second;

        values.erase(value);
    }

    template<class T>
    template<class InputIterator>
    void IntrusiveUnorderedSet<T>::assign(InputIterator first, InputIterator last)
    {
        clear();

        for (auto i = first; i != last; ++i)
            insert(*i);
    }

    template<class T>
    void IntrusiveUnorderedSet<T>::swap(IntrusiveUnorderedSet& other) noexcept
    {
        if (buckets.size() == other.buckets.size())
        {
            std::swap(values, other.values);

            for (auto i = 0; i != buckets.size(); ++i)
                std::swap(buckets[i], other.buckets[i]);
        }
        else
        {
            InitializeBuckets();
            other.InitializeBuckets();

            auto currentEnd = values.end();

            while (!other.values.empty())
            {
                auto& v = *other.values.begin();
                other.values.pop_front();
                insert(v);

                if (currentEnd == end())
                    --currentEnd;
            }

            while (begin() != currentEnd)
            {
                auto& v = *begin();
                values.pop_front();
                other.insert(v);
            }
        }
    }

    template<class T>
    void IntrusiveUnorderedSet<T>::clear()
    {
        values.clear();
        InitializeBuckets();
    }

    template<class T>
    void IntrusiveUnorderedSet<T>::InitializeBuckets()
    {
        std::fill(buckets.begin(), buckets.end(), std::make_pair(values.end(), values.end()));
    }

    template<class T>
    uint32_t IntrusiveUnorderedSet<T>::HashToBucket(const_reference value) const
    {
        return std::hash<value_type>()(value) % buckets.size();
    }

    template<class T>
    bool IntrusiveUnorderedSet<T>::operator==(const IntrusiveUnorderedSet& other) const
    {
        if (values.size() != other.values.size())
            return false;

        for (auto& v : values)
            if (!other.contains(v))
                return false;

        return true;
    }

    template<class T>
    bool IntrusiveUnorderedSet<T>::operator!=(const IntrusiveUnorderedSet& other) const
    {
        return !(*this == other);
    }

    template<class T>
    void swap(IntrusiveUnorderedSet<T>& x, IntrusiveUnorderedSet<T>& y) noexcept
    {
        x.swap(y);
    }
}

#endif
