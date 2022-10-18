#ifndef INFRA_INTRUSIVE_FORWARD_LIST_HPP
#define INFRA_INTRUSIVE_FORWARD_LIST_HPP

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <type_traits>

namespace infra
{
    namespace detail
    {
        template<class T>
        class IntrusiveForwardListNode;

        template<class T>
        class IntrusiveForwardListIterator;
    }

    template<class T>
    class IntrusiveForwardList
    {
    public:
        typedef detail::IntrusiveForwardListNode<T> NodeType;

        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef detail::IntrusiveForwardListIterator<T> iterator;
        typedef detail::IntrusiveForwardListIterator<const T> const_iterator;
        typedef typename std::iterator_traits<iterator>::difference_type difference_type;
        typedef std::size_t size_type;

    public:
        IntrusiveForwardList() = default;
        template<class InputIterator>
        IntrusiveForwardList(InputIterator first, InputIterator last);
        IntrusiveForwardList(const IntrusiveForwardList& other) = delete;
        IntrusiveForwardList(IntrusiveForwardList&& other) noexcept;
        IntrusiveForwardList& operator=(const IntrusiveForwardList& other) = delete;
        IntrusiveForwardList& operator=(IntrusiveForwardList&& other) noexcept;
        ~IntrusiveForwardList() = default;

    public:
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

        const_iterator cbegin() const;
        const_iterator cend() const;

    public:
        bool empty() const;
        bool has_element(const_reference value) const; // Runs in O(n) time

    public:
        reference front();
        const_reference front() const;

    public:
        void push_front(reference value);
        void pop_front();

        void insert_after(const_iterator position, reference value);
        void erase_after(const_reference value);
        void erase_slow(const_reference value); // Runs in O(n) time

        template<class InputIterator>
        void assign(InputIterator first, InputIterator last);

        void swap(IntrusiveForwardList& other) noexcept;

        void clear();

    public:
        bool operator==(const IntrusiveForwardList& other) const;
        bool operator!=(const IntrusiveForwardList& other) const;
        bool operator<(const IntrusiveForwardList& other) const;
        bool operator<=(const IntrusiveForwardList& other) const;
        bool operator>(const IntrusiveForwardList& other) const;
        bool operator>=(const IntrusiveForwardList& other) const;

    private:
        detail::IntrusiveForwardListNode<T>* beginNode { nullptr };
    };

    template<class T>
    void swap(IntrusiveForwardList<T>& x, IntrusiveForwardList<T>& y) noexcept;

    namespace detail
    {
        template<class T>
        class IntrusiveForwardListNode
        {
        protected:
            IntrusiveForwardListNode();
            IntrusiveForwardListNode(const IntrusiveForwardListNode& other);
            IntrusiveForwardListNode& operator=(const IntrusiveForwardListNode& other);
            ~IntrusiveForwardListNode() = default;

        public:
            bool operator==(const IntrusiveForwardListNode<T>& other) const
            {
                return static_cast<const T&>(*this) == static_cast<const T&>(other);
            }

            bool operator!=(const IntrusiveForwardListNode<T>& other) const
            {
                return !(*this == other);
            }

        private:
            template<class>
            friend class infra::detail::IntrusiveForwardListIterator;
            template<class>
            friend class infra::IntrusiveForwardList;

            IntrusiveForwardListNode<T>* next;
        };

        template<class T>
        class IntrusiveForwardListIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            IntrusiveForwardListIterator();
            explicit IntrusiveForwardListIterator(const IntrusiveForwardListNode<typename std::remove_const<T>::type>* node);
            template<class T2>
            IntrusiveForwardListIterator(const IntrusiveForwardListIterator<T2>& other);

            template<class T2>
            IntrusiveForwardListIterator& operator=(const IntrusiveForwardListIterator<T2>& other);

            T& operator*() const;
            T* operator->() const;

            IntrusiveForwardListIterator& operator++();
            IntrusiveForwardListIterator operator++(int);

            template<class T2>
            bool operator==(const IntrusiveForwardListIterator<T2>& other) const;
            template<class T2>
            bool operator!=(const IntrusiveForwardListIterator<T2>& other) const;

        private:
            template<class>
            friend class infra::detail::IntrusiveForwardListIterator;
            template<class>
            friend class infra::IntrusiveForwardList;

            const IntrusiveForwardListNode<typename std::remove_const<T>::type>* mNode;
        };
    }

    ////    Implementation    ////

    template<class T>
    template<class InputIterator>
    IntrusiveForwardList<T>::IntrusiveForwardList(InputIterator first, InputIterator last)
    {
        assign(first, last);
    }

    template<class T>
    IntrusiveForwardList<T>::IntrusiveForwardList(IntrusiveForwardList&& other) noexcept
        : beginNode(other.beginNode)
    {
        other.beginNode = nullptr;
    }

    template<class T>
    IntrusiveForwardList<T>& IntrusiveForwardList<T>::operator=(IntrusiveForwardList&& other) noexcept
    {
        beginNode = other.beginNode;
        other.beginNode = nullptr;

        return *this;
    }

    template<class T>
    typename IntrusiveForwardList<T>::iterator IntrusiveForwardList<T>::begin()
    {
        return iterator(beginNode);
    }

    template<class T>
    typename IntrusiveForwardList<T>::const_iterator IntrusiveForwardList<T>::begin() const
    {
        return const_iterator(beginNode);
    }

    template<class T>
    typename IntrusiveForwardList<T>::iterator IntrusiveForwardList<T>::end()
    {
        return iterator();
    }

    template<class T>
    typename IntrusiveForwardList<T>::const_iterator IntrusiveForwardList<T>::end() const
    {
        return const_iterator();
    }

    template<class T>
    typename IntrusiveForwardList<T>::const_iterator IntrusiveForwardList<T>::cbegin() const
    {
        return begin();
    }

    template<class T>
    typename IntrusiveForwardList<T>::const_iterator IntrusiveForwardList<T>::cend() const
    {
        return end();
    }

    template<class T>
    bool IntrusiveForwardList<T>::empty() const
    {
        return beginNode == nullptr;
    }

    template<class T>
    bool IntrusiveForwardList<T>::has_element(const_reference value) const
    {
        for (const_reference item : *this)
            if (&item == &value)
                return true;

        return false;
    }

    template<class T>
    typename IntrusiveForwardList<T>::reference IntrusiveForwardList<T>::front()
    {
        return static_cast<T&>(*beginNode);
    }

    template<class T>
    typename IntrusiveForwardList<T>::const_reference IntrusiveForwardList<T>::front() const
    {
        return static_cast<T&>(*beginNode);
    }

    template<class T>
    void IntrusiveForwardList<T>::push_front(reference value)
    {
        NodeType& node = value;
        node.next = beginNode;

        beginNode = &node;
    }

    template<class T>
    void IntrusiveForwardList<T>::pop_front()
    {
        beginNode = beginNode->next;
    }

    template<class T>
    void IntrusiveForwardList<T>::insert_after(const_iterator position, reference value)
    {
        NodeType& node = value;
        node.next = const_cast<NodeType*>(position.mNode)->next;
        const_cast<NodeType*>(position.mNode)->next = &node;
    }

    template<class T>
    void IntrusiveForwardList<T>::erase_after(const_reference value)
    {
        NodeType& node = const_cast<reference>(value);
        node.next = node.next->next;
    }

    template<class T>
    void IntrusiveForwardList<T>::erase_slow(const_reference value)
    {
        iterator previous;
        for (iterator index = begin(); index != end(); previous = index++)
            if (&*index == &value)
            {
                if (index != begin())
                    erase_after(*previous);
                else
                    pop_front();
                break;
            }
    }

    template<class T>
    template<class InputIterator>
    void IntrusiveForwardList<T>::assign(InputIterator first, InputIterator last)
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
    void IntrusiveForwardList<T>::swap(IntrusiveForwardList& other) noexcept
    {
        std::swap(beginNode, other.beginNode);
    }

    template<class T>
    void IntrusiveForwardList<T>::clear()
    {
        beginNode = nullptr;
    }

    template<class T>
    bool IntrusiveForwardList<T>::operator==(const IntrusiveForwardList& other) const
    {
        const NodeType* i = beginNode;
        const NodeType* j = other.beginNode;

        while (i != nullptr && j != nullptr)
        {
            if (*i != *j)
                return false;

            i = i->next;
            j = j->next;
        }

        return i == j;
    }

    template<class T>
    bool IntrusiveForwardList<T>::operator!=(const IntrusiveForwardList& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool IntrusiveForwardList<T>::operator<(const IntrusiveForwardList& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    template<class T>
    bool IntrusiveForwardList<T>::operator<=(const IntrusiveForwardList& other) const
    {
        return !(other < *this);
    }

    template<class T>
    bool IntrusiveForwardList<T>::operator>(const IntrusiveForwardList& other) const
    {
        return other < *this;
    }

    template<class T>
    bool IntrusiveForwardList<T>::operator>=(const IntrusiveForwardList& other) const
    {
        return !(*this < other);
    }

    template<class T>
    void swap(IntrusiveForwardList<T>& x, IntrusiveForwardList<T>& y) noexcept
    {
        x.swap(y);
    }

    namespace detail
    {
        template<class T>
        IntrusiveForwardListNode<T>::IntrusiveForwardListNode()
            : next(nullptr)
        {}

        template<class T>
        IntrusiveForwardListNode<T>::IntrusiveForwardListNode(const IntrusiveForwardListNode& other)
            : next(nullptr)
        {}

        template<class T>
        IntrusiveForwardListNode<T>& IntrusiveForwardListNode<T>::operator=(const IntrusiveForwardListNode& other)
        {
            return *this;
        }

        template<class T>
        IntrusiveForwardListIterator<T>::IntrusiveForwardListIterator()
            : mNode(0)
        {}

        template<class T>
        IntrusiveForwardListIterator<T>::IntrusiveForwardListIterator(const IntrusiveForwardListNode<typename std::remove_const<T>::type>* node)
            : mNode(node)
        {}

        template<class T>
        template<class T2>
        IntrusiveForwardListIterator<T>::IntrusiveForwardListIterator(const IntrusiveForwardListIterator<T2>& other)
            : mNode(other.mNode)
        {}

        template<class T>
        template<class T2>
        IntrusiveForwardListIterator<T>& IntrusiveForwardListIterator<T>::operator=(const IntrusiveForwardListIterator<T2>& other)
        {
            mNode = other.mNode;

            return *this;
        }

        template<class T>
        T& IntrusiveForwardListIterator<T>::operator*() const
        {
            return const_cast<T&>(static_cast<const T&>(*mNode));
        }

        template<class T>
        T* IntrusiveForwardListIterator<T>::operator->() const
        {
            return const_cast<T*>(static_cast<const T*>(mNode));
        }

        template<class T>
        IntrusiveForwardListIterator<T>& IntrusiveForwardListIterator<T>::operator++()
        {
            mNode = mNode->next;
            return *this;
        }

        template<class T>
        IntrusiveForwardListIterator<T> IntrusiveForwardListIterator<T>::operator++(int)
        {
            IntrusiveForwardListIterator copy(*this);
            mNode = mNode->next;
            return copy;
        }

        template<class T>
        template<class T2>
        bool IntrusiveForwardListIterator<T>::operator==(const IntrusiveForwardListIterator<T2>& other) const
        {
            return mNode == other.mNode;
        }

        template<class T>
        template<class T2>
        bool IntrusiveForwardListIterator<T>::operator!=(const IntrusiveForwardListIterator<T2>& other) const
        {
            return !(*this == other);
        }
    }
}

#endif
