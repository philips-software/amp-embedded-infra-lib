#ifndef INFRA_INTRUSIVE_BINARY_SEARCH_TREE_HPP
#define INFRA_INTRUSIVE_BINARY_SEARCH_TREE_HPP

// Intrusive binary search tree class, used as base for IntrusiveSet

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <tuple>
#include <type_traits>

namespace infra
{
    template<class T, class Compare = std::less<T>>
    class IntrusiveBinarySearchTree;

    namespace detail
    {
        template<class T>
        class IntrusiveBinarySearchTreeNode
        {
        public:
            IntrusiveBinarySearchTreeNode();
            IntrusiveBinarySearchTreeNode(const IntrusiveBinarySearchTreeNode& other);
            ~IntrusiveBinarySearchTreeNode();

            IntrusiveBinarySearchTreeNode& operator=(const IntrusiveBinarySearchTreeNode& other);

        public:
            T* parent = nullptr;
            T* left = nullptr;
            T* right = nullptr;
            T* previousInOrder = nullptr;
            T* nextInOrder = nullptr;
        };

        template<class T, class Compare>
        class IntrusiveBinarySearchTreeIterator;
    }

    template<class T, class Compare>
    class IntrusiveBinarySearchTree
    {
    public:
        typedef detail::IntrusiveBinarySearchTreeNode<T> NodeType;

        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef detail::IntrusiveBinarySearchTreeIterator<T, Compare> iterator;
        typedef detail::IntrusiveBinarySearchTreeIterator<const T, Compare> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef typename std::iterator_traits<iterator>::difference_type difference_type;
        typedef std::size_t size_type;

    public:
        explicit IntrusiveBinarySearchTree(Compare compare = Compare());
        IntrusiveBinarySearchTree(const IntrusiveBinarySearchTree&) = delete;
        IntrusiveBinarySearchTree(IntrusiveBinarySearchTree&& other);
        IntrusiveBinarySearchTree& operator=(const IntrusiveBinarySearchTree&) = delete;
        IntrusiveBinarySearchTree& operator=(IntrusiveBinarySearchTree&& other);
        ~IntrusiveBinarySearchTree();

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

        iterator find(const_reference value);
        const_iterator find(const_reference value) const;
        bool has_element(const_reference value) const;

        bool invariant_holds() const; // For testing purposes

    public:
        reference front();
        const_reference front() const;
        reference back();
        const_reference back() const;
        reference root();
        const_reference root() const;

    public:
        void insert_in_tree(reference value);
        std::tuple<T*, T*, T**> remove_from_tree(reference value); // Returns the node replacing the removed node, the parent of the replacing node before replacing, and a pointer to the parent's left or right where the old replacing node was
        T* find_in_tree(const_reference value) const;
        T* sibling(const_reference node);
        T*& parent_child_reference(reference node, const_reference child);
        void right_rotate(reference top);
        void left_rotate(reference top);

        void clear();

        void swap(IntrusiveBinarySearchTree& other);

    public:
        bool operator==(const IntrusiveBinarySearchTree& other) const;
        bool operator!=(const IntrusiveBinarySearchTree& other) const;
        bool operator<(const IntrusiveBinarySearchTree& other) const;
        bool operator<=(const IntrusiveBinarySearchTree& other) const;
        bool operator>(const IntrusiveBinarySearchTree& other) const;
        bool operator>=(const IntrusiveBinarySearchTree& other) const;

    protected:
        const T* RootNode() const;

    private:
        void erase_from_order(reference value);

    private:
        Compare compare;
        size_type numberOfNodes = 0;
        T* rootNode = nullptr;
        T* frontNode = nullptr;
        T* backNode = nullptr;
    };

    template<class T, class Compare>
    void swap(IntrusiveBinarySearchTree<T, Compare>& node, IntrusiveBinarySearchTree<T, Compare>& uncle);

    namespace detail
    {
        template<class T, class Compare>
        class IntrusiveBinarySearchTreeIterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            using TreeType = IntrusiveBinarySearchTree<typename std::remove_const<T>::type, Compare>;

            IntrusiveBinarySearchTreeIterator();
            IntrusiveBinarySearchTreeIterator(const T* node, const TreeType& tree);
            template<class T2>
            explicit IntrusiveBinarySearchTreeIterator(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other);

            template<class T2>
            IntrusiveBinarySearchTreeIterator& operator=(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other);

            T& operator*() const;
            T* operator->() const;

            IntrusiveBinarySearchTreeIterator& operator++();
            IntrusiveBinarySearchTreeIterator operator++(int);
            IntrusiveBinarySearchTreeIterator& operator--();
            IntrusiveBinarySearchTreeIterator operator--(int);

            template<class T2>
            bool operator==(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other) const;
            template<class T2>
            bool operator!=(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other) const;

        private:
            template<class, class>
            friend class infra::detail::IntrusiveBinarySearchTreeIterator;
            template<class, class>
            friend class infra::IntrusiveBinarySearchTree;

            const T* node = nullptr;
            const TreeType* tree = nullptr;
        };
    }

    ////    Implementation    ////

    template<class T, class Compare>
    IntrusiveBinarySearchTree<T, Compare>::IntrusiveBinarySearchTree(Compare compare)
        : compare(compare)
    {}

    template<class T, class Compare>
    IntrusiveBinarySearchTree<T, Compare>::IntrusiveBinarySearchTree(IntrusiveBinarySearchTree&& other)
    {
        *this = std::move(other);
    }

    template<class T, class Compare>
    IntrusiveBinarySearchTree<T, Compare>& IntrusiveBinarySearchTree<T, Compare>::operator=(IntrusiveBinarySearchTree&& other)
    {
        clear();

        std::swap(numberOfNodes, other.numberOfNodes);
        std::swap(rootNode, other.rootNode);
        std::swap(frontNode, other.frontNode);
        std::swap(backNode, other.backNode);

        return *this;
    }

    template<class T, class Compare>
    IntrusiveBinarySearchTree<T, Compare>::~IntrusiveBinarySearchTree()
    {
        clear();
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::iterator IntrusiveBinarySearchTree<T, Compare>::begin()
    {
        return iterator(frontNode, *this);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_iterator IntrusiveBinarySearchTree<T, Compare>::begin() const
    {
        return const_iterator(frontNode, *this);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::iterator IntrusiveBinarySearchTree<T, Compare>::end()
    {
        return iterator(nullptr, *this);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_iterator IntrusiveBinarySearchTree<T, Compare>::end() const
    {
        return const_iterator(nullptr, *this);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::reverse_iterator IntrusiveBinarySearchTree<T, Compare>::rbegin()
    {
        return reverse_iterator(end());
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reverse_iterator IntrusiveBinarySearchTree<T, Compare>::rbegin() const
    {
        return const_reverse_iterator(end());
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::reverse_iterator IntrusiveBinarySearchTree<T, Compare>::rend()
    {
        return reverse_iterator(begin());
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reverse_iterator IntrusiveBinarySearchTree<T, Compare>::rend() const
    {
        return const_reverse_iterator(begin());
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_iterator IntrusiveBinarySearchTree<T, Compare>::cbegin() const
    {
        return begin();
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_iterator IntrusiveBinarySearchTree<T, Compare>::cend() const
    {
        return end();
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reverse_iterator IntrusiveBinarySearchTree<T, Compare>::crbegin() const
    {
        return rbegin();
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reverse_iterator IntrusiveBinarySearchTree<T, Compare>::crend() const
    {
        return rend();
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::size_type IntrusiveBinarySearchTree<T, Compare>::size() const
    {
        return numberOfNodes;
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::empty() const
    {
        return numberOfNodes == 0;
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::iterator IntrusiveBinarySearchTree<T, Compare>::find(const_reference value)
    {
        return iterator(find_in_tree(value), *this);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_iterator IntrusiveBinarySearchTree<T, Compare>::find(const_reference value) const
    {
        return const_iterator(find_in_tree(value), *this);
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::has_element(const_reference value) const
    {
        return find_in_tree(value) == &value;
    }

    template<class T, class Compare>
    T* IntrusiveBinarySearchTree<T, Compare>::find_in_tree(const_reference value) const
    {
        T* node = rootNode;

        while (node != nullptr)
        {
            if (node == &value)
                return node;
            if (compare(*node, value))
                node = node->right;
            else
                node = node->left;
        }

        return nullptr;
    }

    template<class T, class Compare>
    T* IntrusiveBinarySearchTree<T, Compare>::sibling(const_reference node)
    {
        if (node.parent->left == &node)
            return node.parent->right;
        else
            return node.parent->left;
    }

    template<class T, class Compare>
    T*& IntrusiveBinarySearchTree<T, Compare>::parent_child_reference(reference parent, const_reference child)
    {
        if (parent.left == &child)
            return parent.left;
        else
            return parent.right;
    }

    template<class T, class Compare>
    void IntrusiveBinarySearchTree<T, Compare>::right_rotate(reference top)
    {
        T* left = top.left;

        top.left = left->right;
        left->right = &top;
        left->parent = top.parent;
        top.parent = left;

        if (top.left != nullptr)
            top.left->parent = &top;

        if (left->parent != nullptr)
            parent_child_reference(*left->parent, top) = left;
        else
            rootNode = left;
    }

    template<class T, class Compare>
    void IntrusiveBinarySearchTree<T, Compare>::left_rotate(reference top)
    {
        T* right = top.right;

        top.right = right->left;
        right->left = &top;
        right->parent = top.parent;
        top.parent = right;

        if (top.right != nullptr)
            top.right->parent = &top;

        if (right->parent != nullptr)
            parent_child_reference(*right->parent, top) = right;
        else
            rootNode = right;
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::invariant_holds() const
    {
        std::size_t nodesDiscovered = 0;

        const T* node = frontNode;

        while (node != nullptr)
        {
            ++nodesDiscovered;

            // A node is the root node if and only if it has no parent
            if ((node->parent == nullptr) != (node == rootNode))
                return false;

            if (node->parent != nullptr)
            {
                if (node->parent->left == node)
                {
                    if (!compare(*node, *node->parent))
                        return false;
                }
                else if (node->parent->right == node)
                {
                    if (!compare(*node->parent, *node))
                        return false;
                }
                else
                    return false;
            }

            if (node->left != nullptr)
            {
                if (node->left->parent != node)
                    return false;
                if (!compare(*node->left, *node))
                    return false;
            }

            if (node->right != nullptr)
            {
                if (node->right->parent != node)
                    return false;
                if (!compare(*node, *node->right))
                    return false;
            }

            // Only the front node has no previousInOrder
            if ((node->previousInOrder == nullptr) != (node == frontNode))
                return false;
            // Only the back node has no nextInOrder
            if ((node->nextInOrder == nullptr) != (node == backNode))
                return false;

            const T* nextInOrder = node;
            if (nextInOrder->right != nullptr)
            {
                nextInOrder = nextInOrder->right;

                while (nextInOrder->left != nullptr)
                    nextInOrder = nextInOrder->left;
            }
            else
            {
                while (nextInOrder->parent && nextInOrder->parent->right == nextInOrder)
                    nextInOrder = nextInOrder->parent;

                if (nextInOrder->parent != nullptr)
                    nextInOrder = nextInOrder->parent;
                else
                    nextInOrder = nullptr;
            }

            if (node->nextInOrder != nextInOrder)
                return false;
            if (nextInOrder && nextInOrder->previousInOrder != node)
                return false;

            if (!nextInOrder && node != backNode)
                return false;

            node = nextInOrder;
        }

        if (nodesDiscovered != numberOfNodes)
            return false;

        return true;
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::reference IntrusiveBinarySearchTree<T, Compare>::front()
    {
        return static_cast<reference>(*frontNode);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reference IntrusiveBinarySearchTree<T, Compare>::front() const
    {
        return static_cast<const_reference>(*frontNode);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::reference IntrusiveBinarySearchTree<T, Compare>::back()
    {
        return static_cast<reference>(*backNode);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reference IntrusiveBinarySearchTree<T, Compare>::back() const
    {
        return static_cast<const_reference>(*backNode);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::reference IntrusiveBinarySearchTree<T, Compare>::root()
    {
        return static_cast<reference>(*rootNode);
    }

    template<class T, class Compare>
    typename IntrusiveBinarySearchTree<T, Compare>::const_reference IntrusiveBinarySearchTree<T, Compare>::root() const
    {
        return static_cast<const_reference>(*rootNode);
    }

    template<class T, class Compare>
    void IntrusiveBinarySearchTree<T, Compare>::insert_in_tree(reference value)
    {
        ++numberOfNodes;

        if (!rootNode)
        {
            rootNode = &value;
            frontNode = &value;
            backNode = &value;
        }
        else
        {
            T* current = rootNode;
            while (true)
            {
                if (compare(value, *current))
                {
                    if (current->left != nullptr)
                        current = current->left;
                    else
                    {
                        current->left = &value;
                        value.parent = current;
                        value.nextInOrder = current;
                        value.previousInOrder = current->previousInOrder;
                        current->previousInOrder = &value;
                        if (value.previousInOrder != nullptr)
                            value.previousInOrder->nextInOrder = &value;
                        else
                            frontNode = &value;
                        break;
                    }
                }
                else
                {
                    if (current->right != nullptr)
                        current = current->right;
                    else
                    {
                        current->right = &value;
                        value.parent = current;
                        value.previousInOrder = current;
                        value.nextInOrder = current->nextInOrder;
                        current->nextInOrder = &value;
                        if (value.nextInOrder != nullptr)
                            value.nextInOrder->previousInOrder = &value;
                        else
                            backNode = &value;
                        break;
                    }
                }
            }
        }
    }

    template<class T, class Compare>
    std::tuple<T*, T*, T**> IntrusiveBinarySearchTree<T, Compare>::remove_from_tree(reference value)
    {
        T* replacement = nullptr;
        T* oldReplacementParent = nullptr;
        T** oldReplacement = nullptr;

        if (!value.left && !value.right)
        {
            if (!value.parent)
            {
                rootNode = nullptr;
                frontNode = nullptr;
                backNode = nullptr;
            }
            else
            {
                oldReplacementParent = &value;
                oldReplacement = &parent_child_reference(*value.parent, value);

                if (value.parent->left == &value)
                {
                    value.parent->left = nullptr;
                    if (&value == frontNode)
                        frontNode = value.parent;
                }
                else
                {
                    value.parent->right = nullptr;
                    if (&value == backNode)
                        backNode = value.parent;
                }
            }
        }
        else if (!value.left)
        {
            replacement = value.right;
            oldReplacementParent = replacement->parent;
            oldReplacement = &parent_child_reference(*replacement->parent, *replacement);

            value.right->parent = value.parent;

            if (!value.parent)
                rootNode = value.right;
            else
                parent_child_reference(*value.parent, value) = value.right;

            if (&value == frontNode)
                frontNode = value.nextInOrder;
        }
        else if (!value.right)
        {
            replacement = value.left;
            oldReplacementParent = replacement->parent;
            oldReplacement = &parent_child_reference(*replacement->parent, *replacement);

            value.left->parent = value.parent;

            if (!value.parent)
                rootNode = value.left;
            else
                parent_child_reference(*value.parent, value) = value.left;

            if (&value == backNode)
                backNode = value.previousInOrder;
        }
        else
        {
            replacement = value.nextInOrder;
            oldReplacementParent = replacement->parent;
            oldReplacement = &parent_child_reference(*replacement->parent, *replacement);

            if (replacement != value.right)
            {
                if (replacement->right != nullptr)
                    replacement->right->parent = replacement->parent;
                replacement->parent->left = replacement->right;

                replacement->right = value.right;
                replacement->right->parent = replacement;
            }

            replacement->parent = value.parent;
            replacement->left = value.left;
            replacement->left->parent = replacement;

            if (!replacement->parent)
                rootNode = replacement;
            else
                parent_child_reference(*replacement->parent, value) = replacement;
        }

        value.parent = nullptr;
        value.left = nullptr;
        value.right = nullptr;

        erase_from_order(value);
        --numberOfNodes;

        return std::make_tuple(replacement, oldReplacementParent, oldReplacement);
    }

    template<class T, class Compare>
    const T* IntrusiveBinarySearchTree<T, Compare>::RootNode() const
    {
        return rootNode;
    }

    template<class T, class Compare>
    void IntrusiveBinarySearchTree<T, Compare>::erase_from_order(reference value)
    {
        if (value.previousInOrder != nullptr)
            value.previousInOrder->nextInOrder = value.nextInOrder;
        if (value.nextInOrder != nullptr)
            value.nextInOrder->previousInOrder = value.previousInOrder;

        value.nextInOrder = nullptr;
        value.previousInOrder = nullptr;
    }

    template<class T, class Compare>
    void IntrusiveBinarySearchTree<T, Compare>::clear()
    {
        while (!empty())
            remove_from_tree(*rootNode);
    }

    template<class T, class Compare>
    void IntrusiveBinarySearchTree<T, Compare>::swap(IntrusiveBinarySearchTree& other)
    {
        std::swap(numberOfNodes, other.numberOfNodes);
        std::swap(rootNode, other.rootNode);
        std::swap(frontNode, other.frontNode);
        std::swap(backNode, other.backNode);
        std::swap(compare, other.compare);
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::operator==(const IntrusiveBinarySearchTree& other) const
    {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::operator!=(const IntrusiveBinarySearchTree& other) const
    {
        return !(*this == other);
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::operator<(const IntrusiveBinarySearchTree& other) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end(), compare);
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::operator<=(const IntrusiveBinarySearchTree& other) const
    {
        return !(other < *this);
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::operator>(const IntrusiveBinarySearchTree& other) const
    {
        return other < *this;
    }

    template<class T, class Compare>
    bool IntrusiveBinarySearchTree<T, Compare>::operator>=(const IntrusiveBinarySearchTree& other) const
    {
        return !(*this < other);
    }

    template<class T, class Compare>
    void swap(IntrusiveBinarySearchTree<T, Compare>& node, IntrusiveBinarySearchTree<T, Compare>& uncle)
    {
        node.swap(uncle);
    }

    namespace detail
    {
        template<class T>
        IntrusiveBinarySearchTreeNode<T>::IntrusiveBinarySearchTreeNode()
        {}

        template<class T>
        IntrusiveBinarySearchTreeNode<T>::IntrusiveBinarySearchTreeNode(const IntrusiveBinarySearchTreeNode& other)
        {}

        template<class T>
        IntrusiveBinarySearchTreeNode<T>::~IntrusiveBinarySearchTreeNode()
        {
            assert(parent == nullptr);
            assert(left == nullptr);
            assert(right == nullptr);
            assert(nextInOrder == nullptr);
            assert(previousInOrder == nullptr);
        }

        template<class T>
        IntrusiveBinarySearchTreeNode<T>& IntrusiveBinarySearchTreeNode<T>::operator=(const IntrusiveBinarySearchTreeNode& other)
        {
            return *this;
        }

        template<class T, class Compare>
        IntrusiveBinarySearchTreeIterator<T, Compare>::IntrusiveBinarySearchTreeIterator()
        {}

        template<class T, class Compare>
        IntrusiveBinarySearchTreeIterator<T, Compare>::IntrusiveBinarySearchTreeIterator(const T* node, const TreeType& tree)
            : node(node)
            , tree(&tree)
        {}

        template<class T, class Compare>
        template<class T2>
        IntrusiveBinarySearchTreeIterator<T, Compare>::IntrusiveBinarySearchTreeIterator(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other)
            : node(other.node)
            , tree(other.tree)
        {}

        template<class T, class Compare>
        template<class T2>
        IntrusiveBinarySearchTreeIterator<T, Compare>& IntrusiveBinarySearchTreeIterator<T, Compare>::operator=(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other)
        {
            node = other.node;
            tree = other.tree;

            return *this;
        }

        template<class T, class Compare>
        T& IntrusiveBinarySearchTreeIterator<T, Compare>::operator*() const
        {
            return const_cast<T&>(static_cast<const T&>(*node));
        }

        template<class T, class Compare>
        T* IntrusiveBinarySearchTreeIterator<T, Compare>::operator->() const
        {
            return const_cast<T*>(static_cast<const T*>(node));
        }

        template<class T, class Compare>
        IntrusiveBinarySearchTreeIterator<T, Compare>& IntrusiveBinarySearchTreeIterator<T, Compare>::operator++()
        {
            node = node->nextInOrder;
            return *this;
        }

        template<class T, class Compare>
        IntrusiveBinarySearchTreeIterator<T, Compare> IntrusiveBinarySearchTreeIterator<T, Compare>::operator++(int)
        {
            IntrusiveBinarySearchTreeIterator copy(*this);
            ++*this;
            return copy;
        }

        template<class T, class Compare>
        IntrusiveBinarySearchTreeIterator<T, Compare>& IntrusiveBinarySearchTreeIterator<T, Compare>::operator--()
        {
            if (node != nullptr)
                node = node->previousInOrder;
            else
                node = &tree->back();

            return *this;
        }

        template<class T, class Compare>
        IntrusiveBinarySearchTreeIterator<T, Compare> IntrusiveBinarySearchTreeIterator<T, Compare>::operator--(int)
        {
            IntrusiveBinarySearchTreeIterator copy(*this);
            --*this;
            return copy;
        }

        template<class T, class Compare>
        template<class T2>
        bool IntrusiveBinarySearchTreeIterator<T, Compare>::operator==(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other) const
        {
            return node == other.node && tree == other.tree;
        }

        template<class T, class Compare>
        template<class T2>
        bool IntrusiveBinarySearchTreeIterator<T, Compare>::operator!=(const IntrusiveBinarySearchTreeIterator<T2, Compare>& other) const
        {
            return !(*this == other);
        }
    }

}

#endif
