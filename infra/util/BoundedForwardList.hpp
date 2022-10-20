#ifndef INFRA_BOUNDED_FORWARD_LIST_HPP
#define INFRA_BOUNDED_FORWARD_LIST_HPP

//  The BoundedForwardList provides a singly-linked list similar to std::forward_list, however,
//  it is limited in the amount of values that can be pushed in the list. Moreover, nodes can
//  be moved from one std::forward_list to another in constant time, but in BoundedForwardList
//  this is done in linear time, since nodes are copied from one BoundedForwardList to another.
//  Node storage is specifically coupled to a BoundedForwardList.
//  The usecase for using BoundedForwardList is that values can be inserted in the middle of
//  the list in constant time.

#include "infra/util/MemoryRange.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/StaticStorage.hpp"
#include "infra/util/WithStorage.hpp"
#include <array>
#include <iterator>

namespace infra
{
    namespace detail
    {
        template<class T>
        struct BoundedForwardListNode
        {
            StaticStorage<T> storage;
            BoundedForwardListNode<T>* next;
        };

        template<class T>
        class BoundedForwardListIterator;
    }

    template<class T>
    class BoundedForwardList
    {
    public:
        template<std::size_t Max>
        using WithMaxSize = infra::WithStorage<BoundedForwardList<T>, std::array<StaticStorage<detail::BoundedForwardListNode<T>>, Max>>;

        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef detail::BoundedForwardListIterator<T> iterator;
        typedef detail::BoundedForwardListIterator<const T> const_iterator;
        typedef typename std::iterator_traits<iterator>::difference_type difference_type;
        typedef std::size_t size_type;

    public:
        BoundedForwardList(const BoundedForwardList& other) = delete;
        explicit BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage);
        BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, size_type n, const value_type& value = value_type());
        template<class InputIterator>
        BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, InputIterator first, InputIterator last);
        BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, std::initializer_list<T> initializerList);
        BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, const BoundedForwardList& other);
        BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, BoundedForwardList&& other);
        ~BoundedForwardList();

        BoundedForwardList& operator=(const BoundedForwardList& other);
        BoundedForwardList& operator=(BoundedForwardList&& other);
        void AssignFromStorage(const BoundedForwardList& other);
        void AssignFromStorage(BoundedForwardList&& other);

    public:
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

        const_iterator cbegin() const;
        const_iterator cend() const;

    public:
        size_type size() const;
        size_type max_size() const;
        bool empty() const;
        bool full() const;

    public:
        value_type& front();
        const value_type& front() const;

    public:
        void push_front(const value_type& value);
        void push_front(value_type&& value);
        void pop_front();

        void insert_after(iterator position, const value_type& value);
        void insert_after(iterator position, value_type&& value);

        template<class InputIterator>
        void assign(InputIterator first, InputIterator last);
        void assign(size_type n, const value_type& value);
        template<class InputIterator>
        void move_from_range(InputIterator first, InputIterator last);

        void erase_slow(iterator position);
        void erase_all_after(iterator position);

        void swap(BoundedForwardList& other);

        void clear();

        template<class... Args>
        void emplace_front(Args&&... args);

    public:
        bool operator==(const BoundedForwardList& other) const;
        bool operator!=(const BoundedForwardList& other) const;
        bool operator<(const BoundedForwardList& other) const;
        bool operator<=(const BoundedForwardList& other) const;
        bool operator>(const BoundedForwardList& other) const;
        bool operator>=(const BoundedForwardList& other) const;

    private:
        typedef detail::BoundedForwardListNode<typename std::remove_const<T>::type> NodeType;

    private:
        NodeType* AllocateNode();

    private:
        infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage;
        size_type numAllocated = 0;
        size_type numInitialized = 0;
        NodeType* firstNode = nullptr;
        NodeType* freeList = nullptr;
    };

    template<class T>
    void swap(BoundedForwardList<T>& x, BoundedForwardList<T>& y);

    namespace detail
    {
        template<class T>
        class BoundedForwardListIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

        private:
            typedef detail::BoundedForwardListNode<typename std::remove_const<T>::type> NodeType;

        public:
            BoundedForwardListIterator();
            explicit BoundedForwardListIterator(NodeType* node);
            template<class T2>
            BoundedForwardListIterator(const BoundedForwardListIterator<T2>& other);
            template<class T2>
            BoundedForwardListIterator& operator=(const BoundedForwardListIterator<T2>& other);

            T& operator*() const;
            T* operator->() const;

            NodeType& node();

            BoundedForwardListIterator& operator++();
            BoundedForwardListIterator operator++(int);

            template<class T2>
            bool operator==(const BoundedForwardListIterator<T2>& other) const;
            template<class T2>
            bool operator!=(const BoundedForwardListIterator<T2>& other) const;

        private:
            template<class>
            friend class BoundedForwardListIterator;

            NodeType* mNode;
        };
    }

    ////    Implementation    ////

    template<class T>
    BoundedForwardList<T>::BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage)
        : storage(storage)
    {}

    template<class T>
    BoundedForwardList<T>::BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, size_type n, const value_type& value)
        : storage(storage)
    {
        assign(n, value);
    }

    template<class T>
    template<class InputIterator>
    BoundedForwardList<T>::BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, InputIterator first, InputIterator last)
        : storage(storage)
    {
        assign(first, last);
    }

    template<class T>
    BoundedForwardList<T>::BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, std::initializer_list<T> initializerList)
        : storage(storage)
    {
        assign(initializerList.begin(), initializerList.end());
    }

    template<class T>
    BoundedForwardList<T>::BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, const BoundedForwardList& other)
        : storage(storage)
    {
        assign(other.begin(), other.end());
    }

    template<class T>
    BoundedForwardList<T>::BoundedForwardList(infra::MemoryRange<infra::StaticStorage<detail::BoundedForwardListNode<T>>> storage, BoundedForwardList&& other)
        : storage(storage)
    {
        move_from_range(other.begin(), other.end());
        other.clear();
    }

    template<class T>
    BoundedForwardList<T>::~BoundedForwardList()
    {
        clear();
    }

    template<class T>
    BoundedForwardList<T>& BoundedForwardList<T>::operator=(const BoundedForwardList& other)
    {
        if (this != &other)
        {
            clear();

            assign(other.begin(), other.end());
        }

        return *this;
    }

    template<class T>
    BoundedForwardList<T>& BoundedForwardList<T>::operator=(BoundedForwardList&& other)
    {
        clear();
        move_from_range(other.begin(), other.end());

        return *this;
    }

    template<class T>
    void BoundedForwardList<T>::AssignFromStorage(const BoundedForwardList& other)
    {
        *this = other;
    }

    template<class T>
    void BoundedForwardList<T>::AssignFromStorage(BoundedForwardList&& other)
    {
        *this = std::move(other);
    }

    template<class T>
    typename BoundedForwardList<T>::iterator BoundedForwardList<T>::begin()
    {
        return iterator(firstNode);
    }

    template<class T>
    typename BoundedForwardList<T>::const_iterator BoundedForwardList<T>::begin() const
    {
        return iterator(firstNode);
    }

    template<class T>
    typename BoundedForwardList<T>::iterator BoundedForwardList<T>::end()
    {
        return iterator();
    }

    template<class T>
    typename BoundedForwardList<T>::const_iterator BoundedForwardList<T>::end() const
    {
        return const_iterator();
    }

    template<class T>
    typename BoundedForwardList<T>::const_iterator BoundedForwardList<T>::cbegin() const
    {
        return const_iterator(firstNode);
    }

    template<class T>
    typename BoundedForwardList<T>::const_iterator BoundedForwardList<T>::cend() const
    {
        return const_iterator();
    }

    template<class T>
    typename BoundedForwardList<T>::size_type BoundedForwardList<T>::size() const
    {
        return numAllocated;
    }

    template<class T>
    typename BoundedForwardList<T>::size_type BoundedForwardList<T>::max_size() const
    {
        return storage.size();
    }

    template<class T>
    bool BoundedForwardList<T>::empty() const
    {
        return numAllocated == 0;
    }

    template<class T>
    bool BoundedForwardList<T>::full() const
    {
        return numAllocated == storage.size();
    }

    template<class T>
    typename BoundedForwardList<T>::value_type& BoundedForwardList<T>::front()
    {
        return *firstNode->storage;
    }

    template<class T>
    const typename BoundedForwardList<T>::value_type& BoundedForwardList<T>::front() const
    {
        return *firstNode->storage;
    }

    template<class T>
    void BoundedForwardList<T>::push_front(const value_type& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(value);

        node->next = firstNode;
        firstNode = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedForwardList<T>::push_front(value_type&& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(std::move(value));

        node->next = firstNode;
        firstNode = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedForwardList<T>::pop_front()
    {
        firstNode->storage.Destruct();

        NodeType* oldStart = firstNode;
        firstNode = firstNode->next;
        oldStart->next = freeList;
        freeList = oldStart;

        --numAllocated;
    }

    template<class T>
    void BoundedForwardList<T>::insert_after(iterator position, const value_type& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(value);

        node->next = position.node().next;
        position.node().next = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedForwardList<T>::insert_after(iterator position, value_type&& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(std::move(value));

        node->next = position.node().next;
        position.node().next = node;

        ++numAllocated;
    }

    template<class T>
    template<class InputIterator>
    void BoundedForwardList<T>::assign(InputIterator first, InputIterator last)
    {
        clear();

        iterator current;
        while (first != last)
        {
            if (empty())
            {
                push_front(*first);
                current = begin();
            }
            else
            {
                insert_after(current, *first);
                ++current;
            }
            ++first;
        }
    }

    template<class T>
    void BoundedForwardList<T>::assign(size_type n, const value_type& value)
    {
        clear();

        for (size_type i = 0; i != n; ++i)
            push_front(value);
    }

    template<class T>
    template<class InputIterator>
    void BoundedForwardList<T>::move_from_range(InputIterator first, InputIterator last)
    {
        clear();

        iterator current;
        while (first != last)
        {
            if (empty())
            {
                push_front(std::move(*first));
                current = begin();
            }
            else
            {
                insert_after(current, std::move(*first));
                ++current;
            }
            ++first;
        }
    }

    template<class T>
    void BoundedForwardList<T>::erase_slow(iterator position)
    {
        NodeType* node = &position.node();

        if (firstNode == node)
            firstNode = node->next;
        else
        {
            NodeType* previous = firstNode;
            while (previous->next != node)
                previous = previous->next;

            previous->next = node->next;
        }

        node->next = freeList;
        freeList = node;

        --numAllocated;
    }

    template<class T>
    void BoundedForwardList<T>::erase_all_after(iterator position)
    {
        NodeType* node = position.node().next;
        position.node().next = 0;

        while (node != nullptr)
        {
            node->storage.Destruct();
            NodeType* oldFreeList = freeList;
            freeList = node;
            node = node->next;
            freeList->next = oldFreeList;

            --numAllocated;
        }
    }

    template<class T>
    void BoundedForwardList<T>::swap(BoundedForwardList& other)
    {
        using std::swap;

        iterator i1 = begin();
        iterator i2 = other.begin();
        iterator i1old;
        iterator i2old;

        while (i1 != end() && i2 != other.end())
        {
            swap(*i1, *i2);
            i1old = i1;
            i2old = i2;
            ++i1;
            ++i2;
        }

        if (i1 != end())
        {
            iterator cutOff1 = i1old;

            while (i1 != end())
            {
                other.insert_after(i2old, std::move(*i1));
                ++i2old;
                ++i1;
            }

            erase_all_after(cutOff1);
        }

        if (i2 != other.end())
        {
            iterator cutOff2 = i2old;

            while (i2 != other.end())
            {
                insert_after(i1old, std::move(*i2));
                ++i1old;
                ++i2;
            }

            other.erase_all_after(cutOff2);
        }
    }

    template<class T>
    void BoundedForwardList<T>::clear()
    {
        while (firstNode != nullptr)
            pop_front();
    }

    template<class T>
    template<class... Args>
    void BoundedForwardList<T>::emplace_front(Args&&... args)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(std::forward<Args>(args)...);

        node->next = firstNode;
        firstNode = node;

        ++numAllocated;
    }

    template<class T>
    bool BoundedForwardList<T>::operator==(const BoundedForwardList& other) const
    {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }

    template<class T>
    bool BoundedForwardList<T>::operator!=(const BoundedForwardList& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool BoundedForwardList<T>::operator<(const BoundedForwardList<T>& other) const
    {
        const_iterator i1 = begin();
        const_iterator i2 = other.begin();

        while (i1 != end() && i2 != other.end())
        {
            if (*i1 < *i2)
                return true;

            ++i1;
            ++i2;
        }

        return size() < other.size();
    }

    template<class T>
    bool BoundedForwardList<T>::operator<=(const BoundedForwardList<T>& other) const
    {
        return !(other < *this);
    }

    template<class T>
    bool BoundedForwardList<T>::operator>(const BoundedForwardList<T>& other) const
    {
        return other < *this;
    }

    template<class T>
    bool BoundedForwardList<T>::operator>=(const BoundedForwardList<T>& other) const
    {
        return !(*this < other);
    }

    template<class T>
    typename BoundedForwardList<T>::NodeType* BoundedForwardList<T>::AllocateNode()
    {
        NodeType* result;

        if (freeList != nullptr)
        {
            result = freeList;
            freeList = freeList->next;
            return result;
        }

        really_assert(numInitialized != storage.size());

        result = &*storage[numInitialized];
        ++numInitialized;
        return result;
    }

    template<class T>
    void swap(BoundedForwardList<T>& x, BoundedForwardList<T>& y)
    {
        x.swap(y);
    }

    namespace detail
    {

        template<class T>
        BoundedForwardListIterator<T>::BoundedForwardListIterator()
            : mNode(0)
        {}

        template<class T>
        BoundedForwardListIterator<T>::BoundedForwardListIterator(NodeType* node)
            : mNode(node)
        {}

        template<class T>
        template<class T2>
        BoundedForwardListIterator<T>::BoundedForwardListIterator(const BoundedForwardListIterator<T2>& other)
            : mNode(other.mNode)
        {}

        template<class T>
        template<class T2>
        BoundedForwardListIterator<T>& BoundedForwardListIterator<T>::operator=(const BoundedForwardListIterator<T2>& other)
        {
            mNode = other.mNode;

            return *this;
        }

        template<class T>
        T& BoundedForwardListIterator<T>::operator*() const
        {
            return *mNode->storage;
        }

        template<class T>
        T* BoundedForwardListIterator<T>::operator->() const
        {
            return &*mNode->storage;
        }

        template<class T>
        typename BoundedForwardListIterator<T>::NodeType& BoundedForwardListIterator<T>::node()
        {
            really_assert(mNode != 0);
            return *mNode;
        }

        template<class T>
        BoundedForwardListIterator<T>& BoundedForwardListIterator<T>::operator++()
        {
            mNode = mNode->next;
            return *this;
        }

        template<class T>
        BoundedForwardListIterator<T> BoundedForwardListIterator<T>::operator++(int)
        {
            BoundedForwardListIterator copy(*this);
            mNode = mNode->next;
            return copy;
        }

        template<class T>
        template<class T2>
        bool BoundedForwardListIterator<T>::operator==(const BoundedForwardListIterator<T2>& other) const
        {
            return mNode == other.mNode;
        }

        template<class T>
        template<class T2>
        bool BoundedForwardListIterator<T>::operator!=(const BoundedForwardListIterator<T2>& other) const
        {
            return !(*this == other);
        }
    }
}

#endif
