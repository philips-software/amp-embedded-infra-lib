#ifndef INFRA_BOUNDED_PRIORITY_QUEUE_HPP
#define INFRA_BOUNDED_PRIORITY_QUEUE_HPP

//  BoundedPriorityQueue is similar to std::priority_queue, except that it can contain a limited number of elements

#include "infra/util/BoundedVector.hpp"
#include <functional>

namespace infra
{
    template<class T, std::size_t Max, class Compare = std::less<T>>
    class BoundedPriorityQueue
    {
    public:
        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef std::size_t size_type;
        typedef typename BoundedVector<T>::template WithMaxSize<Max>::iterator iterator;
        typedef typename BoundedVector<T>::template WithMaxSize<Max>::const_iterator const_iterator;
        typedef typename BoundedVector<T>::template WithMaxSize<Max>::reverse_iterator reverse_iterator;
        typedef typename BoundedVector<T>::template WithMaxSize<Max>::const_reverse_iterator const_reverse_iterator;

    public:
        explicit BoundedPriorityQueue(const Compare& comp = Compare());
        template<class InputIterator>
        BoundedPriorityQueue(InputIterator first, InputIterator last, const Compare& comp = Compare());
        BoundedPriorityQueue(const BoundedPriorityQueue& other);
        BoundedPriorityQueue(BoundedPriorityQueue&& other);

        BoundedPriorityQueue& operator=(const BoundedPriorityQueue& other);
        BoundedPriorityQueue& operator=(BoundedPriorityQueue&& other);

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
        bool full() const;
        size_type size() const;
        reference top();
        const_reference top() const;
        const value_type& operator[](size_type position) const;

    public:
        void push(const_reference value);
        void push(value_type&& value);
        void pop();
        void clear();
        void erase(iterator position);

        template<class... Args>
        void emplace(Args&&... args);

        void swap(BoundedPriorityQueue& x);

    private:
        typename BoundedVector<value_type>::template WithMaxSize<Max> values;
        Compare compare;
    };

    ////    Implementation    ////

    template<class RandomAccessIterator, class Compare>
    void erase_heap(RandomAccessIterator begin, RandomAccessIterator end, RandomAccessIterator position, Compare compare)
    {
        std::size_t size = std::distance(begin, end);
        if (size > 1)
        {
            while (true)
            {
                size_t index = std::distance(begin, position);
                RandomAccessIterator parent = index > 0 ? begin + (index - 1) / 2 : end;
                RandomAccessIterator child1 = index * 2 + 1 < size - 1 ? begin + index * 2 + 1 : end;
                RandomAccessIterator child2 = index * 2 + 2 < size - 1 ? begin + index * 2 + 2 : end;

                if (parent != end && compare(*parent, *std::prev(end)))
                {
                    *position = std::move(*parent);
                    position = parent;
                }
                else if (child1 != end && compare(*std::prev(end), *child1))
                {
                    if (child2 != end && compare(*child1, *child2))
                    {
                        *position = std::move(*child2);
                        position = child2;
                    }
                    else
                    {
                        *position = std::move(*child1);
                        position = child1;
                    }
                }
                else if (child2 != end && compare(*std::prev(end), *child2))
                {
                    *position = std::move(*child2);
                    position = child2;
                }
                else
                {
                    *position = std::move(*std::prev(end));
                    break;
                }
            }
        }
    }

    template<class T, std::size_t Max, class Compare>
    BoundedPriorityQueue<T, Max, Compare>::BoundedPriorityQueue(const Compare& comp)
        : compare(comp)
    {}

    template<class T, std::size_t Max, class Compare>
    template<class InputIterator>
    BoundedPriorityQueue<T, Max, Compare>::BoundedPriorityQueue(InputIterator first, InputIterator last, const Compare& comp)
        : values(first, last)
        , compare(comp)
    {
        std::make_heap(values.begin(), values.end(), compare);
    }

    template<class T, std::size_t Max, class Compare>
    BoundedPriorityQueue<T, Max, Compare>::BoundedPriorityQueue(const BoundedPriorityQueue& other)
        : values(other.values)
        , compare(other.compare)
    {}

    template<class T, std::size_t Max, class Compare>
    BoundedPriorityQueue<T, Max, Compare>::BoundedPriorityQueue(BoundedPriorityQueue&& other)
        : values(std::move(other.values))
        , compare(std::move(other.compare))
    {}

    template<class T, std::size_t Max, class Compare>
    BoundedPriorityQueue<T, Max, Compare>& BoundedPriorityQueue<T, Max, Compare>::operator=(const BoundedPriorityQueue& other)
    {
        values = other.values;
        compare = other.compare;

        return *this;
    }

    template<class T, std::size_t Max, class Compare>
    BoundedPriorityQueue<T, Max, Compare>& BoundedPriorityQueue<T, Max, Compare>::operator=(BoundedPriorityQueue&& other)
    {
        values = std::move(other.values);
        compare = std::move(other.compare);

        return *this;
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::iterator BoundedPriorityQueue<T, Max, Compare>::begin()
    {
        return values.begin();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_iterator BoundedPriorityQueue<T, Max, Compare>::begin() const
    {
        return values.begin();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::iterator BoundedPriorityQueue<T, Max, Compare>::end()
    {
        return values.end();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_iterator BoundedPriorityQueue<T, Max, Compare>::end() const
    {
        return values.end();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::reverse_iterator BoundedPriorityQueue<T, Max, Compare>::rbegin()
    {
        return values.rbegin();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_reverse_iterator BoundedPriorityQueue<T, Max, Compare>::rbegin() const
    {
        return values.rbegin();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::reverse_iterator BoundedPriorityQueue<T, Max, Compare>::rend()
    {
        return values.rend();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_reverse_iterator BoundedPriorityQueue<T, Max, Compare>::rend() const
    {
        return values.rend();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_iterator BoundedPriorityQueue<T, Max, Compare>::cbegin() const
    {
        return values.cbegin();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_iterator BoundedPriorityQueue<T, Max, Compare>::cend() const
    {
        return values.cend();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_reverse_iterator BoundedPriorityQueue<T, Max, Compare>::crbegin() const
    {
        return values.crbegin();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_reverse_iterator BoundedPriorityQueue<T, Max, Compare>::crend() const
    {
        return values.crend();
    }

    template<class T, std::size_t Max, class Compare>
    bool BoundedPriorityQueue<T, Max, Compare>::empty() const
    {
        return values.empty();
    }

    template<class T, std::size_t Max, class Compare>
    bool BoundedPriorityQueue<T, Max, Compare>::full() const
    {
        return values.full();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::size_type BoundedPriorityQueue<T, Max, Compare>::size() const
    {
        return values.size();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::reference BoundedPriorityQueue<T, Max, Compare>::top()
    {
        return values.front();
    }

    template<class T, std::size_t Max, class Compare>
    typename BoundedPriorityQueue<T, Max, Compare>::const_reference BoundedPriorityQueue<T, Max, Compare>::top() const
    {
        return values.front();
    }

    template<class T, std::size_t Max, class Compare>
    const typename BoundedPriorityQueue<T, Max, Compare>::value_type& BoundedPriorityQueue<T, Max, Compare>::operator[](size_type position) const
    {
        return values[position];
    }

    template<class T, std::size_t Max, class Compare>
    void BoundedPriorityQueue<T, Max, Compare>::push(const_reference value)
    {
        values.push_back(value);
        std::push_heap(values.begin(), values.end(), compare);
    }

    template<class T, std::size_t Max, class Compare>
    void BoundedPriorityQueue<T, Max, Compare>::push(value_type&& value)
    {
        values.push_back(std::move(value));
        std::push_heap(values.begin(), values.end(), compare);
    }

    template<class T, std::size_t Max, class Compare>
    void BoundedPriorityQueue<T, Max, Compare>::pop()
    {
        std::pop_heap(values.begin(), values.end(), compare);
        values.pop_back();
    }

    template<class T, std::size_t Max, class Compare>
    void BoundedPriorityQueue<T, Max, Compare>::clear()
    {
        values.clear();
    }

    template<class T, std::size_t Max, class Compare>
    void BoundedPriorityQueue<T, Max, Compare>::erase(iterator position)
    {
        erase_heap(values.begin(), values.end(), position, compare);
        values.pop_back();
    }

    template<class T, std::size_t Max, class Compare>
    template<class... Args>
    void BoundedPriorityQueue<T, Max, Compare>::emplace(Args&&... args)
    {
        values.emplace_back(std::forward<Args>(args)...);
        std::push_heap(values.begin(), values.end(), compare);
    }

    template<class T, std::size_t Max, class Compare>
    void BoundedPriorityQueue<T, Max, Compare>::swap(BoundedPriorityQueue& other)
    {
        using std::swap;
        swap(values, other.values);
        swap(compare, other.compare);
    }
}

#endif
