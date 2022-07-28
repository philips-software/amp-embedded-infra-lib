#ifndef INFRA_INTRUSIVE_PRIORITY_QUEUE_HPP
#define INFRA_INTRUSIVE_PRIORITY_QUEUE_HPP

// Key features of an intrusive priority queue:
// - Elements inserted must descend from IntrusivePriority::NodeType
// - Insertions and deletions are done in O(log(n)) time
// - Finding the largest element takes O(1) time

#include <cstdlib>
#include <functional>
#include <utility>
#include <cassert>

namespace infra
{
    namespace detail
    {
        template<class T>
        class IntrusivePriorityQueueNode;
    }

    template<class T, class Compare = std::less<T>>
    class IntrusivePriorityQueue
    {
    public:
        typedef detail::IntrusivePriorityQueueNode<T> NodeType;

        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef std::size_t size_type;

    public:
        explicit IntrusivePriorityQueue(const Compare& comp = Compare());
        template <class InputIterator>
            IntrusivePriorityQueue(InputIterator first, InputIterator last, const Compare& comp = Compare());
        IntrusivePriorityQueue(const IntrusivePriorityQueue&) = delete;
        IntrusivePriorityQueue(IntrusivePriorityQueue&& other);
        IntrusivePriorityQueue& operator=(const IntrusivePriorityQueue&) = delete;
        IntrusivePriorityQueue& operator=(IntrusivePriorityQueue&& other);

    public:
        bool empty() const;
        size_type size() const;
        T& top();
        const T& top() const;

    public:
        void push(const T& value);
        void pop();
        void clear();
        void erase(const T& value);

        void swap(IntrusivePriorityQueue& x);

    private:
        void SwapChildAndParentNodes(T& child, T& parent);
        void InsertAsLast(T& value);
        T& LastNode() const;

    private:
        T* topNode = nullptr;
        std::size_t numberOfElements = 0;
        Compare compare;
    };

    namespace detail
    {
        template<class T>
        class IntrusivePriorityQueueNode
        {
        public:
            IntrusivePriorityQueueNode();
            IntrusivePriorityQueueNode(const IntrusivePriorityQueueNode& other);

            IntrusivePriorityQueueNode& operator=(const IntrusivePriorityQueueNode& other);

        private:
            template<class, class>
                friend class infra::IntrusivePriorityQueue;

            T* up;
            T* left;
            T* right;
        };
    }

    ////    Implementation    ////

    template<class T, class Compare>
    IntrusivePriorityQueue<T, Compare>::IntrusivePriorityQueue(const Compare& comp)
        : compare(comp)
    {}

    template<class T, class Compare>
    template <class InputIterator>
    IntrusivePriorityQueue<T, Compare>::IntrusivePriorityQueue(InputIterator first, InputIterator last, const Compare& comp)
        : compare(comp)
    {
        for (; first != last; ++first)
            push(*first);
    }

    template<class T, class Compare>
    IntrusivePriorityQueue<T, Compare>::IntrusivePriorityQueue(IntrusivePriorityQueue&& other)
        : topNode(other.topNode)
        , numberOfElements(other.numberOfElements)
        , compare(std::move(other.compare))
    {
        other.topNode = nullptr;
        other.numberOfElements = 0;
    }

    template<class T, class Compare>
    IntrusivePriorityQueue<T, Compare>& IntrusivePriorityQueue<T, Compare>::operator=(IntrusivePriorityQueue&& other)
    {
        compare = std::move(other.compare);
        topNode = other.topNode;
        numberOfElements = other.numberOfElements;
        other.topNode = nullptr;
        other.numberOfElements = 0;

        return *this;
    }

    template<class T, class Compare>
    bool IntrusivePriorityQueue<T, Compare>::empty() const
    {
        return numberOfElements == 0;
    }

    template<class T, class Compare>
    typename IntrusivePriorityQueue<T, Compare>::size_type IntrusivePriorityQueue<T, Compare>::size() const
    {
        return numberOfElements;
    }

    template<class T, class Compare>
    T& IntrusivePriorityQueue<T, Compare>::top()
    {
        return *topNode;
    }

    template<class T, class Compare>
    const T& IntrusivePriorityQueue<T, Compare>::top() const
    {
        return *topNode;
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::SwapChildAndParentNodes(T& child, T& parent)
    {
        T* parentUp = parent.up;
        if (parentUp != nullptr)
        {
            if (parentUp->left == &parent)
                parentUp->left = &child;
            else
                parentUp->right = &child;
        }

        bool leftRotate = &child == parent.left;

        T* otherChild = leftRotate ? parent.right : parent.left;

        parent.left = child.left;
        parent.right = child.right;

        if (parent.left != nullptr)
            parent.left->up = &parent;
        if (parent.right != nullptr)
            parent.right->up = &parent;

        parent.up = &child;
        if (otherChild != nullptr)
            otherChild->up = &child;

        child.left = leftRotate ? &parent : otherChild;
        child.right = leftRotate ? otherChild : &parent;

        child.up = parentUp;

        if (topNode == &parent)
            topNode = &child;
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::push(const T& value)
    {
        T& node = const_cast<T&>(value);
        InsertAsLast(node);

        while (node.up && compare(*node.up, node))
            SwapChildAndParentNodes(node, *node.up);
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::pop()
    {
        erase(*topNode);
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::clear()
    {
        topNode = nullptr;
        numberOfElements = 0;
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::erase(const T& position)
    {
        T& erasingNode = const_cast<T&>(position);

        // First, move erasingNode all the way down in the tree, maintaining heap invariant except for erasingNode
        while (true)
        {
            T* shiftUpNode = nullptr;

            if (erasingNode.left && erasingNode.right)
            {
                if (compare(*erasingNode.left, *erasingNode.right))
                    shiftUpNode = erasingNode.right;
                else
                    shiftUpNode = erasingNode.left;
            }
            else if (erasingNode.left != nullptr)
                shiftUpNode = erasingNode.left;
            else
                shiftUpNode = erasingNode.right;

            if (shiftUpNode != nullptr)
                SwapChildAndParentNodes(*shiftUpNode, erasingNode);
            else
                break;
        }

        // Then, replace erasingNode by lastNode
        T& last = LastNode();
        if (&last != &erasingNode)
        {
            assert(last.up);        // if last.up does not exist, then the tree holds only one element, and &last == &erasingNode
            assert(erasingNode.up); // if erasingNode.up does not exist, then the tree holds only one element, and &last == &erasingNode
            if (last.up->left == &last)
                last.up->left = nullptr;
            else
                last.up->right = nullptr;

            last.up = erasingNode.up;
            last.left = erasingNode.left;
            last.right = erasingNode.right;

            if (last.up->left == &erasingNode)
                last.up->left = &last;
            else
                last.up->right = &last;

            if (last.right != nullptr)
                last.right->up = &last;
            if (last.left != nullptr)
                last.left->up = &last;

            // Finally, move lastNode up in the tree, restoring the heap invariant
            while (true)
            {
                if (last.up == nullptr)
                    break;
                if (compare(last, *last.up))
                    break;

                SwapChildAndParentNodes(last, *last.up);
            }
        }
        else
        {
            if (!last.up)
                topNode = nullptr;
            else
            {
                if (last.up->left == &last)
                    last.up->left = nullptr;
                else
                    last.up->right = nullptr;
            }
        }

        --numberOfElements;
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::swap(IntrusivePriorityQueue& other)
    {
        using std::swap;
        swap(topNode, other.topNode);
        swap(numberOfElements, other.numberOfElements);
    }

    template<class T, class Compare>
    void IntrusivePriorityQueue<T, Compare>::InsertAsLast(T& value)
    {
        value.right = nullptr;
        value.left = nullptr;

        if (!topNode)
        {
            ++numberOfElements;
            topNode = &value;
            value.up = nullptr;
        }
        else
        {
            ++numberOfElements;
            T* node = topNode;

            std::size_t direction = 1;
            while (direction <= numberOfElements)
                direction <<= 1;
            direction >>= 2;

            while (direction != 1)
            {
                if ((numberOfElements & direction) != 0)
                    node = node->right;
                else
                    node = node->left;

                direction >>= 1;
            }

            if ((numberOfElements & direction) != 0)
                node->right = &value;
            else
                node->left = &value;

            value.up = node;
        }
    }

    template<class T, class Compare>
    T& IntrusivePriorityQueue<T, Compare>::LastNode() const
    {
        T* node = topNode;

        std::size_t direction = 1;
        while (direction <= numberOfElements)
            direction <<= 1;
        direction >>= 2;

        while (direction != 0)
        {
            if ((numberOfElements & direction) != 0)
                node = node->right;
            else
                node = node->left;

            direction >>= 1;
        }

        return *node;
    }

    namespace detail
    {
        template<class T>
        IntrusivePriorityQueueNode<T>::IntrusivePriorityQueueNode()
            : up(nullptr)
            , left(nullptr)
            , right(nullptr)
        {}

        template<class T>
        IntrusivePriorityQueueNode<T>::IntrusivePriorityQueueNode(const IntrusivePriorityQueueNode& other)
            : up(nullptr)
            , left(nullptr)
            , right(nullptr)
        {}

        template<class T>
        IntrusivePriorityQueueNode<T>& IntrusivePriorityQueueNode<T>::operator=(const IntrusivePriorityQueueNode& other)
        {
            return *this;
        }
    }
}

#endif
