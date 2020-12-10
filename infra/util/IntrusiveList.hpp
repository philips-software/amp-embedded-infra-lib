#ifndef INFRA_INTRUSIVE_LIST_HPP
#define INFRA_INTRUSIVE_LIST_HPP

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <type_traits>

namespace infra
{
    template<class T>
    class IntrusiveList;

    namespace detail
    {
        template<class T>
        class IntrusiveListIterator;

        template<class T>
        class IntrusiveListNode
        {
        protected:
            IntrusiveListNode();

        public:
            IntrusiveListNode(const IntrusiveListNode& other);
            IntrusiveListNode& operator=(const IntrusiveListNode& other);
            ~IntrusiveListNode() = default;

        private:
            template<class>
                friend class infra::detail::IntrusiveListIterator;
            template<class>
                friend class infra::IntrusiveList;

            IntrusiveListNode<T>* next;
            IntrusiveListNode<T>* previous;
        };
    }

    template<class T>
    class IntrusiveList
    {
    public:
        typedef detail::IntrusiveListNode<T> NodeType;

        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef detail::IntrusiveListIterator<T> iterator;
        typedef detail::IntrusiveListIterator<const T> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef typename std::iterator_traits<iterator>::difference_type difference_type;
        typedef std::size_t size_type;

    public:
        IntrusiveList();
        template<class InputIterator>
            IntrusiveList(InputIterator first, InputIterator last);
        IntrusiveList(const IntrusiveList&) = delete;
        IntrusiveList(IntrusiveList&& other);
        IntrusiveList& operator=(const IntrusiveList&) = delete;
        IntrusiveList& operator=(IntrusiveList&& other);
        ~IntrusiveList();

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
        bool empty() const;
        bool has_element(const_reference value) const;  // Runs in O(n) time

    public:
        reference front();
        const_reference front() const;
        reference back();
        const_reference back() const;

    public:
        void push_front(const_reference value);
        void push_back(const_reference value);
        void pop_front();
        void pop_back();

        void insert(const_iterator position, const_reference value);
        void erase(const_reference value);

        template<class InputIterator>
            void assign(InputIterator first, InputIterator last);

        void swap(IntrusiveList& other);

        void clear();

    public:
        bool operator==(const IntrusiveList& other) const;
        bool operator!=(const IntrusiveList& other) const;
        bool operator<(const IntrusiveList& other) const;
        bool operator<=(const IntrusiveList& other) const;
        bool operator>(const IntrusiveList& other) const;
        bool operator>=(const IntrusiveList& other) const;

    private:
        size_type numberOfElements;
        NodeType* beginNode;
        NodeType endNode;
    };

    template<class T>
        void swap(IntrusiveList<T>& x, IntrusiveList<T>& y);

    namespace detail
    {
        template<class T>
        class IntrusiveListIterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            IntrusiveListIterator();
            explicit IntrusiveListIterator(const IntrusiveListNode<typename std::remove_const<T>::type>* node);
            template<class T2>                                                                                  //TICS !INT#001
                IntrusiveListIterator(const IntrusiveListIterator<T2>& other);

            template<class T2>
                IntrusiveListIterator& operator=(const IntrusiveListIterator<T2>& other);

            T& operator*() const;
            T* operator->() const;

            IntrusiveListIterator& operator++();
            IntrusiveListIterator operator++(int);
            IntrusiveListIterator& operator--();
            IntrusiveListIterator operator--(int);

            template<class T2>
                bool operator==(const IntrusiveListIterator<T2>& other) const;
            template<class T2>
                bool operator!=(const IntrusiveListIterator<T2>& other) const;

        private:
            template<class>
                friend class infra::detail::IntrusiveListIterator;
            template<class>
                friend class infra::IntrusiveList;

            const IntrusiveListNode<typename std::remove_const<T>::type>* node;
        };
    }

    ////    Implementation    ////

    template<class T>
    IntrusiveList<T>::IntrusiveList()
        : numberOfElements(0)
        , beginNode(&endNode)
    {}

    template<class T>
    template<class InputIterator>
    IntrusiveList<T>::IntrusiveList(InputIterator first, InputIterator last)
        : numberOfElements(0)
        , beginNode(&endNode)
    {
        assign(first, last);
    }

    template<class T>
    IntrusiveList<T>::IntrusiveList(IntrusiveList&& other)
        : numberOfElements(0)
        , beginNode(&endNode)
    {
        *this = std::move(other);
    }

    template<class T>
    IntrusiveList<T>& IntrusiveList<T>::operator=(IntrusiveList&& other)
    {
        clear();
        numberOfElements = other.numberOfElements;
        if (other.beginNode != &other.endNode)
            beginNode = other.beginNode;
        endNode.previous = other.endNode.previous;
        if (endNode.previous != nullptr)
            endNode.previous->next = &endNode;
        other.beginNode = &other.endNode;
        other.endNode.previous = nullptr;
        other.numberOfElements = 0;

        return *this;
    }

    template<class T>
    IntrusiveList<T>::~IntrusiveList()
    {
        clear();
    }

    template<class T>
    typename IntrusiveList<T>::iterator IntrusiveList<T>::begin()
    {
        return iterator(beginNode);
    }

    template<class T>
    typename IntrusiveList<T>::const_iterator IntrusiveList<T>::begin() const
    {
        return const_iterator(beginNode);
    }

    template<class T>
    typename IntrusiveList<T>::iterator IntrusiveList<T>::end()
    {
        return iterator(&endNode);
    }

    template<class T>
    typename IntrusiveList<T>::const_iterator IntrusiveList<T>::end() const
    {
        return const_iterator(&endNode);
    }

    template<class T>
    typename IntrusiveList<T>::reverse_iterator IntrusiveList<T>::rbegin()
    {
        return reverse_iterator(end());
    }

    template<class T>
    typename IntrusiveList<T>::const_reverse_iterator IntrusiveList<T>::rbegin() const
    {
        return const_reverse_iterator(end());
    }

    template<class T>
    typename IntrusiveList<T>::reverse_iterator IntrusiveList<T>::rend()
    {
        return reverse_iterator(begin());
    }

    template<class T>
    typename IntrusiveList<T>::const_reverse_iterator IntrusiveList<T>::rend() const
    {
        return const_reverse_iterator(begin());
    }

    template<class T>
    typename IntrusiveList<T>::const_iterator IntrusiveList<T>::cbegin() const
    {
        return begin();
    }

    template<class T>
    typename IntrusiveList<T>::const_iterator IntrusiveList<T>::cend() const
    {
        return end();
    }

    template<class T>
    typename IntrusiveList<T>::const_reverse_iterator IntrusiveList<T>::crbegin() const
    {
        return rbegin();
    }

    template<class T>
    typename IntrusiveList<T>::const_reverse_iterator IntrusiveList<T>::crend() const
    {
        return rend();
    }

    template<class T>
    typename IntrusiveList<T>::size_type IntrusiveList<T>::size() const
    {
        return numberOfElements;
    }

    template<class T>
    bool IntrusiveList<T>::empty() const
    {
        return numberOfElements == 0;
    }

    template<class T>
    bool IntrusiveList<T>::has_element(const_reference value) const
    {
        for (const_reference item: *this)
            if (&item == &value)
                return true;

        return false;
    }

    template<class T>
    typename IntrusiveList<T>::reference IntrusiveList<T>::front()
    {
        return static_cast<T&>(*beginNode);
    }

    template<class T>
    typename IntrusiveList<T>::const_reference IntrusiveList<T>::front() const
    {
        return static_cast<T&>(*beginNode);
    }

    template<class T>
    typename IntrusiveList<T>::reference IntrusiveList<T>::back()
    {
        return static_cast<T&>(*endNode.previous);
    }

    template<class T>
    typename IntrusiveList<T>::const_reference IntrusiveList<T>::back() const
    {
        return static_cast<T&>(*endNode.previous);
    }

    template<class T>
    void IntrusiveList<T>::push_front(const_reference value)
    {
        NodeType& node = const_cast<reference>(value);
        node.next = beginNode;
        node.previous = nullptr;

        if (node.next != nullptr)
            node.next->previous = &node;

        beginNode = &node;
        ++numberOfElements;
    }

    template<class T>
    void IntrusiveList<T>::push_back(const_reference value)
    {
        NodeType& node = const_cast<reference>(value);
        node.next = &endNode;
        node.previous = endNode.previous;

        if (node.previous != nullptr)
            node.previous->next = &node;
        else
            beginNode = &node;

        endNode.previous = &node;
        ++numberOfElements;
    }

    template<class T>
    void IntrusiveList<T>::pop_front()
    {
        NodeType* erasingNode = beginNode;
        beginNode = beginNode->next;
        beginNode->previous = nullptr;
        erasingNode->next = nullptr;
        --numberOfElements;
    }

    template<class T>
    void IntrusiveList<T>::pop_back()
    {
        NodeType* erasingNode = endNode.previous;
        endNode.previous = endNode.previous->previous;
        if (endNode.previous != nullptr)
            endNode.previous->next = &endNode;
        erasingNode->previous = nullptr;
        --numberOfElements;
    }

    template<class T>
    void IntrusiveList<T>::insert(const_iterator position, const_reference value)
    {
        NodeType& node = const_cast<reference>(value);
        node.previous = position.node->previous;
        node.next = const_cast<NodeType*>(position.node);
        const_cast<NodeType*>(position.node)->previous = &node;
        if (node.previous != nullptr)
            node.previous->next = &node;
        else
            beginNode = &node;
        ++numberOfElements;
    }

    template<class T>
    void IntrusiveList<T>::erase(const_reference value)
    {
        NodeType& node = const_cast<reference>(value);
        if (node.previous != nullptr)
            node.previous->next = node.next;
        else if (beginNode != &node)
            return;
        else
            beginNode = node.next;

        node.next->previous = node.previous;
        node.previous = nullptr;
        node.next = nullptr;
        --numberOfElements;
    }

    template<class T>
    template<class InputIterator>
    void IntrusiveList<T>::assign(InputIterator first, InputIterator last)
    {
        clear();
        for (; first != last; ++first)
            push_back(*first);
    }

    template<class T>
    void IntrusiveList<T>::swap(IntrusiveList& other)
    {
        using std::swap;
        std::swap(endNode, other.endNode);
        std::swap(beginNode, other.beginNode);
        std::swap(numberOfElements, other.numberOfElements);

        if (endNode.previous != nullptr)
            endNode.previous->next = &endNode;
        else
            beginNode = &endNode;

        if (other.endNode.previous != nullptr)
            other.endNode.previous->next = &other.endNode;
        else
            other.beginNode = &other.endNode;
    }

    template<class T>
    void IntrusiveList<T>::clear()
    {
        while (!empty())
            pop_front();
    }

    template<class T>
    bool IntrusiveList<T>::operator==(const IntrusiveList& other) const
    {
        return size() == other.size()
            && std::equal(begin(), end(), other.begin());
    }

    template<class T>
    bool IntrusiveList<T>::operator!=(const IntrusiveList& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool IntrusiveList<T>::operator<(const IntrusiveList& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    template<class T>
    bool IntrusiveList<T>::operator<=(const IntrusiveList& other) const
    {
        return !(other < *this);
    }

    template<class T>
    bool IntrusiveList<T>::operator>(const IntrusiveList& other) const
    {
        return other < *this;
    }

    template<class T>
    bool IntrusiveList<T>::operator>=(const IntrusiveList& other) const
    {
        return !(*this < other);
    }

    template<class T>
    void swap(IntrusiveList<T>& x, IntrusiveList<T>& y)
    {
        x.swap(y);
    }

    namespace detail
    {
        template<class T>
        IntrusiveListNode<T>::IntrusiveListNode()
            : next(nullptr)
            , previous(nullptr)
        {}

        template<class T>
        IntrusiveListNode<T>::IntrusiveListNode(const IntrusiveListNode& other)
            : next(nullptr)
            , previous(nullptr)
        {}

        template<class T>
        IntrusiveListNode<T>& IntrusiveListNode<T>::operator=(const IntrusiveListNode& other)
        {
            return *this;
        }

        template<class T>
        IntrusiveListIterator<T>::IntrusiveListIterator()
            : node(nullptr)
        {}

        template<class T>
        IntrusiveListIterator<T>::IntrusiveListIterator(const IntrusiveListNode<typename std::remove_const<T>::type>* node)
            : node(node)
        {}

        template<class T>
        template<class T2>
        IntrusiveListIterator<T>::IntrusiveListIterator(const IntrusiveListIterator<T2>& other)
            : node(other.node)
        {}

        template<class T>
        template<class T2>
        IntrusiveListIterator<T>& IntrusiveListIterator<T>::operator=(const IntrusiveListIterator<T2>& other)
        {
            node = other.node;

            return *this;
        }

        template<class T>
        T& IntrusiveListIterator<T>::operator*() const
        {
            return const_cast<T&>(static_cast<const T&>(*node));
        }

        template<class T>
        T* IntrusiveListIterator<T>::operator->() const
        {
            return const_cast<T*>(static_cast<const T*>(node));
        }

        template<class T>
        IntrusiveListIterator<T>& IntrusiveListIterator<T>::operator++()
        {
            node = node->next;
            return *this;
        }

        template<class T>
        IntrusiveListIterator<T> IntrusiveListIterator<T>::operator++(int)
        {
            IntrusiveListIterator copy(*this);
            node = node->next;
            return copy;
        }

        template<class T>
        IntrusiveListIterator<T>& IntrusiveListIterator<T>::operator--()
        {
            node = node->previous;
            return *this;
        }

        template<class T>
        IntrusiveListIterator<T> IntrusiveListIterator<T>::operator--(int)
        {
            IntrusiveListIterator copy(*this);
            node = node->previous;
            return copy;
        }

        template<class T>
        template<class T2>
        bool IntrusiveListIterator<T>::operator==(const IntrusiveListIterator<T2>& other) const
        {
            return node == other.node;
        }

        template<class T>
        template<class T2>
        bool IntrusiveListIterator<T>::operator!=(const IntrusiveListIterator<T2>& other) const
        {
            return !(*this == other);
        }
    }
}

#endif
