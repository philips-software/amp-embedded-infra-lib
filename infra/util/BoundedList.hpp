#ifndef INFRA_BOUNDED_LIST_HPP
#define INFRA_BOUNDED_LIST_HPP

//  The BoundedList provides a doubly-linked list similar to std::list, however,
//  it is limited in the amount of values that can be pushed in the list. Moreover, nodes can
//  be moved from one std::list to another in constant time, but in BoundedList
//  this is done in linear time, since nodes are copied from one BoundedList to another.
//  Node storage is specifically coupled to a BoundedList.
//  The usecase for using BoundedList is that values can be inserted in the middle of
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
        struct BoundedListNode
        {
            StaticStorage<T> storage;
            BoundedListNode<T>* next = nullptr;
            BoundedListNode<T>* previous = nullptr;
        };

        template<class T, class U>
        class BoundedListIterator;
    }

    template<class T>
    class BoundedList
    {
    public:
        template<std::size_t Max>
        using WithMaxSize = infra::WithStorage<BoundedList<T>, std::array<StaticStorage<detail::BoundedListNode<T>>, Max>>;

        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = detail::BoundedListIterator<T, T>;
        using const_iterator = detail::BoundedListIterator<const T, T>;
        using difference_type = typename std::iterator_traits<iterator>::difference_type;
        using size_type = std::size_t;

    public:
        BoundedList(const BoundedList& other) = delete;
        explicit BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage);
        BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, size_type n, const value_type& value = value_type());
        template<class InputIterator>
        BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, InputIterator first, InputIterator last);
        BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, std::initializer_list<T> initializerList);
        BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, const BoundedList& other);
        BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, BoundedList&& other) noexcept;
        ~BoundedList();

        BoundedList& operator=(const BoundedList& other);
        BoundedList& operator=(BoundedList&& other) noexcept;
        void AssignFromStorage(const BoundedList& other);
        void AssignFromStorage(BoundedList&& other) noexcept;

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
        value_type& back();
        const value_type& back() const;

    public:
        void push_front(const value_type& value);
        void push_front(value_type&& value);
        void pop_front();

        void push_back(const value_type& value);
        void push_back(value_type&& value);
        void pop_back();

        void insert(iterator position, const value_type& value);
        void insert(iterator position, value_type&& value);

        template<class InputIterator>
        void assign(InputIterator first, InputIterator last);
        void assign(size_type n, const value_type& value);
        template<class InputIterator>
        void move_from_range(InputIterator first, InputIterator last);

        void erase(iterator position);
        void erase_all_after(iterator position);
        void remove(reference value);

        void swap(BoundedList& other) noexcept;

        void clear();

        template<class... Args>
        void emplace_front(Args&&... args);
        template<class... Args>
        void emplace_back(Args&&... args);

    public:
        bool operator==(const BoundedList& other) const;
        bool operator!=(const BoundedList& other) const;
        bool operator<(const BoundedList& other) const;
        bool operator<=(const BoundedList& other) const;
        bool operator>(const BoundedList& other) const;
        bool operator>=(const BoundedList& other) const;

    private:
        using NodeType = detail::BoundedListNode<T>;

    private:
        NodeType* AllocateNode();

    private:
        infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage;
        size_type numAllocated = 0;
        size_type numInitialized = 0;
        NodeType* firstNode = nullptr;
        NodeType* lastNode = nullptr;
        NodeType* freeList = nullptr;
    };

    template<class T>
    void swap(BoundedList<T>& x, BoundedList<T>& y) noexcept;

    namespace detail
    {
        template<class T, class U>
        class BoundedListIterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

        private:
            using NodeType = BoundedListNode<U>;
            using ContainerType = BoundedList<U>;

        public:
            BoundedListIterator() = default;
            BoundedListIterator(NodeType* node, const ContainerType* container);
            template<class T2>
            BoundedListIterator(const BoundedListIterator<T2, U>& other);
            template<class T2>
            BoundedListIterator& operator=(const BoundedListIterator<T2, U>& other);

            T& operator*() const;
            T* operator->() const;

            BoundedListIterator& operator++();
            BoundedListIterator operator++(int);
            BoundedListIterator& operator--();
            BoundedListIterator operator--(int);

            template<class T2>
            bool operator==(const BoundedListIterator<T2, U>& other) const;
            template<class T2>
            bool operator!=(const BoundedListIterator<T2, U>& other) const;

            NodeType* node = nullptr;
            const ContainerType* container = nullptr;
        };
    }

    ////    Implementation    ////

    template<class T>
    BoundedList<T>::BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage)
        : storage(storage)
    {}

    template<class T>
    BoundedList<T>::BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, size_type n, const value_type& value)
        : storage(storage)
    {
        assign(n, value);
    }

    template<class T>
    template<class InputIterator>
    BoundedList<T>::BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, InputIterator first, InputIterator last)
        : storage(storage)
    {
        assign(first, last);
    }

    template<class T>
    BoundedList<T>::BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, std::initializer_list<T> initializerList)
        : storage(storage)
    {
        assign(initializerList.begin(), initializerList.end());
    }

    template<class T>
    BoundedList<T>::BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, const BoundedList& other)
        : storage(storage)
    {
        assign(other.begin(), other.end());
    }

    template<class T>
    BoundedList<T>::BoundedList(infra::MemoryRange<infra::StaticStorage<detail::BoundedListNode<T>>> storage, BoundedList&& other) noexcept
        : storage(storage)
    {
        move_from_range(other.begin(), other.end());
        other.clear();
    }

    template<class T>
    BoundedList<T>::~BoundedList()
    {
        clear();
    }

    template<class T>
    BoundedList<T>& BoundedList<T>::operator=(const BoundedList& other)
    {
        if (this != &other)
        {
            clear();

            assign(other.begin(), other.end());
        }

        return *this;
    }

    template<class T>
    BoundedList<T>& BoundedList<T>::operator=(BoundedList&& other) noexcept
    {
        clear();
        move_from_range(other.begin(), other.end());

        return *this;
    }

    template<class T>
    void BoundedList<T>::AssignFromStorage(const BoundedList& other)
    {
        *this = other;
    }

    template<class T>
    void BoundedList<T>::AssignFromStorage(BoundedList&& other) noexcept
    {
        *this = std::move(other);
    }

    template<class T>
    typename BoundedList<T>::iterator BoundedList<T>::begin()
    {
        return iterator(firstNode, this);
    }

    template<class T>
    typename BoundedList<T>::const_iterator BoundedList<T>::begin() const
    {
        return iterator(firstNode, this);
    }

    template<class T>
    typename BoundedList<T>::iterator BoundedList<T>::end()
    {
        return iterator(nullptr, this);
    }

    template<class T>
    typename BoundedList<T>::const_iterator BoundedList<T>::end() const
    {
        return const_iterator(nullptr, this);
    }

    template<class T>
    typename BoundedList<T>::const_iterator BoundedList<T>::cbegin() const
    {
        return const_iterator(firstNode, this);
    }

    template<class T>
    typename BoundedList<T>::const_iterator BoundedList<T>::cend() const
    {
        return const_iterator(nullptr, this);
    }

    template<class T>
    typename BoundedList<T>::size_type BoundedList<T>::size() const
    {
        return numAllocated;
    }

    template<class T>
    typename BoundedList<T>::size_type BoundedList<T>::max_size() const
    {
        return storage.size();
    }

    template<class T>
    bool BoundedList<T>::empty() const
    {
        return numAllocated == 0;
    }

    template<class T>
    bool BoundedList<T>::full() const
    {
        return numAllocated == storage.size();
    }

    template<class T>
    typename BoundedList<T>::value_type& BoundedList<T>::front()
    {
        return *firstNode->storage;
    }

    template<class T>
    const typename BoundedList<T>::value_type& BoundedList<T>::front() const
    {
        return *firstNode->storage;
    }

    template<class T>
    typename BoundedList<T>::value_type& BoundedList<T>::back()
    {
        return *lastNode->storage;
    }

    template<class T>
    const typename BoundedList<T>::value_type& BoundedList<T>::back() const
    {
        return *lastNode->storage;
    }

    template<class T>
    void BoundedList<T>::push_front(const value_type& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(value);

        node->next = firstNode;
        node->previous = nullptr;

        if (firstNode != nullptr)
            firstNode->previous = node;
        else
            lastNode = node;

        firstNode = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedList<T>::push_front(value_type&& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(std::move(value));

        node->next = firstNode;
        node->previous = nullptr;

        if (firstNode != nullptr)
            firstNode->previous = node;
        else
            lastNode = node;

        firstNode = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedList<T>::pop_front()
    {
        firstNode->storage.Destruct();

        NodeType* oldStart = firstNode;
        firstNode = firstNode->next;

        if (firstNode != nullptr)
            firstNode->previous = nullptr;
        else
            lastNode = nullptr;

        oldStart->next = freeList;
        freeList = oldStart;

        --numAllocated;
    }

    template<class T>
    void BoundedList<T>::push_back(const value_type& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(value);

        node->previous = lastNode;
        node->next = nullptr;

        if (lastNode != nullptr)
            lastNode->next = node;
        else
            firstNode = node;

        lastNode = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedList<T>::push_back(value_type&& value)
    {
        NodeType* node = AllocateNode();
        node->storage.Construct(std::move(value));

        node->previous = lastNode;
        node->next = nullptr;

        if (lastNode != nullptr)
            lastNode->next = node;
        else
            firstNode = node;

        lastNode = node;

        ++numAllocated;
    }

    template<class T>
    void BoundedList<T>::pop_back()
    {
        lastNode->storage.Destruct();

        NodeType* oldEnd = lastNode;
        lastNode = lastNode->previous;

        if (lastNode != nullptr)
            lastNode->next = nullptr;
        else
            firstNode = nullptr;

        oldEnd->next = freeList;
        freeList = oldEnd;

        --numAllocated;
    }

    template<class T>
    void BoundedList<T>::insert(iterator position, const value_type& value)
    {
        if (position != end())
        {
            NodeType* node = AllocateNode();
            node->storage.Construct(value);

            node->next = position.node;

            if (node->next != nullptr)
            {
                node->previous = node->next->previous;
                node->next->previous = node;
            }
            else
            {
                node->previous = lastNode;
                lastNode = node;
            }

            if (node->previous != nullptr)
                node->previous->next = node;
            else
                firstNode = node;

            ++numAllocated;
        }
        else
            push_back(value);
    }

    template<class T>
    void BoundedList<T>::insert(iterator position, value_type&& value)
    {
        if (position != end())
        {
            NodeType* node = AllocateNode();
            node->storage.Construct(std::move(value));

            node->next = position.node;

            if (node->next != nullptr)
            {
                node->previous = node->next->previous;
                node->next->previous = node;
            }
            else
            {
                node->previous = lastNode;
                lastNode = node;
            }

            if (node->previous != nullptr)
                node->previous->next = node;
            else
                firstNode = node;

            ++numAllocated;
        }
        else
            push_back(std::move(value));
    }

    template<class T>
    template<class InputIterator>
    void BoundedList<T>::assign(InputIterator first, InputIterator last)
    {
        clear();

        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    template<class T>
    void BoundedList<T>::assign(size_type n, const value_type& value)
    {
        clear();

        for (size_type i = 0; i != n; ++i)
            push_back(value);
    }

    template<class T>
    template<class InputIterator>
    void BoundedList<T>::move_from_range(InputIterator first, InputIterator last)
    {
        clear();

        while (first != last)
        {
            push_back(std::move(*first));
            ++first;
        }
    }

    template<class T>
    void BoundedList<T>::erase(iterator position)
    {
        NodeType* node = position.node;
        NodeType* next = node->next;
        NodeType* previous = node->previous;

        if (next != nullptr)
            next->previous = previous;
        else
            lastNode = previous;

        if (previous != nullptr)
            previous->next = next;
        else
            firstNode = next;

        node->storage.Destruct();
        node->next = freeList;
        freeList = node;

        --numAllocated;
    }

    template<class T>
    void BoundedList<T>::erase_all_after(iterator position)
    {
        NodeType* node = position.node->next;
        position.node->next = nullptr;
        lastNode = position.node;

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
    void BoundedList<T>::remove(reference value)
    {
        erase(iterator(reinterpret_cast<NodeType*>(&value), this));
    }

    template<class T>
    void BoundedList<T>::swap(BoundedList& other) noexcept
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
            while (i1 != end())
            {
                other.push_back(std::move(*i1));
                ++i1;
            }

            erase_all_after(i1old);
        }

        if (i2 != other.end())
        {
            while (i2 != other.end())
            {
                push_back(std::move(*i2));
                ++i2;
            }

            other.erase_all_after(i2old);
        }
    }

    template<class T>
    void BoundedList<T>::clear()
    {
        while (!empty())
            pop_back();
    }

    template<class T>
    template<class... Args>
    void BoundedList<T>::emplace_front(Args&&... args)
    {
        NodeType* node = AllocateNode();
        static_cast<detail::BoundedListNode<T>*>(node)->storage.Construct(std::forward<Args>(args)...);

        node->next = firstNode;
        node->previous = nullptr;

        if (firstNode != nullptr)
            firstNode->previous = node;
        else
            lastNode = node;

        firstNode = node;

        ++numAllocated;
    }

    template<class T>
    template<class... Args>
    void BoundedList<T>::emplace_back(Args&&... args)
    {
        NodeType* node = AllocateNode();
        static_cast<detail::BoundedListNode<T>*>(node)->storage.Construct(std::forward<Args>(args)...);

        node->previous = lastNode;
        node->next = nullptr;

        if (lastNode != nullptr)
            lastNode->next = node;
        else
            firstNode = node;

        lastNode = node;

        ++numAllocated;
    }

    template<class T>
    bool BoundedList<T>::operator==(const BoundedList& other) const
    {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }

    template<class T>
    bool BoundedList<T>::operator!=(const BoundedList& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool BoundedList<T>::operator<(const BoundedList<T>& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    template<class T>
    bool BoundedList<T>::operator<=(const BoundedList<T>& other) const
    {
        return !(other < *this);
    }

    template<class T>
    bool BoundedList<T>::operator>(const BoundedList<T>& other) const
    {
        return other < *this;
    }

    template<class T>
    bool BoundedList<T>::operator>=(const BoundedList<T>& other) const
    {
        return !(*this < other);
    }

    template<class T>
    typename BoundedList<T>::NodeType* BoundedList<T>::AllocateNode()
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
    void swap(BoundedList<T>& x, BoundedList<T>& y) noexcept
    {
        x.swap(y);
    }

    namespace detail
    {
        template<class T, class U>
        BoundedListIterator<T, U>::BoundedListIterator(NodeType* node, const ContainerType* container)
            : node(node)
            , container(container)
        {}

        template<class T, class U>
        template<class T2>
        BoundedListIterator<T, U>::BoundedListIterator(const BoundedListIterator<T2, U>& other)
            : node(other.node)
            , container(other.container)
        {}

        template<class T, class U>
        template<class T2>
        BoundedListIterator<T, U>& BoundedListIterator<T, U>::operator=(const BoundedListIterator<T2, U>& other)
        {
            node = other.node;
            container = other.container;

            return *this;
        }

        template<class T, class U>
        T& BoundedListIterator<T, U>::operator*() const
        {
            return *node->storage;
        }

        template<class T, class U>
        T* BoundedListIterator<T, U>::operator->() const
        {
            return &*node->storage;
        }

        template<class T, class U>
        BoundedListIterator<T, U>& BoundedListIterator<T, U>::operator++()
        {
            node = node->next;
            return *this;
        }

        template<class T, class U>
        BoundedListIterator<T, U> BoundedListIterator<T, U>::operator++(int)
        {
            BoundedListIterator copy(*this);
            ++*this;
            return copy;
        }

        template<class T, class U>
        BoundedListIterator<T, U>& BoundedListIterator<T, U>::operator--()
        {
            if (node != nullptr)
                node = node->previous;
            else
                node = reinterpret_cast<NodeType*>(const_cast<T*>(&container->back()));

            return *this;
        }

        template<class T, class U>
        BoundedListIterator<T, U> BoundedListIterator<T, U>::operator--(int)
        {
            BoundedListIterator copy(*this);
            --*this;
            return copy;
        }

        template<class T, class U>
        template<class T2>
        bool BoundedListIterator<T, U>::operator==(const BoundedListIterator<T2, U>& other) const
        {
            return node == other.node;
        }

        template<class T, class U>
        template<class T2>
        bool BoundedListIterator<T, U>::operator!=(const BoundedListIterator<T2, U>& other) const
        {
            return !(*this == other);
        }
    }
}

#endif
