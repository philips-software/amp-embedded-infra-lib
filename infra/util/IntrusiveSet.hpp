#ifndef INFRA_INTRUSIVE_SET_HPP
#define INFRA_INTRUSIVE_SET_HPP

// Key features of an intrusive set:
// - Elements inserted must descend from IntrusiveSet::NodeType
// - The set keeps all elements sorted
// - Insertions and deletions are done in O(log(n)) time
// - Finding an element takes O(log(n)) time

#include "infra/util/IntrusiveBinarySearchTree.hpp"
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <type_traits>

namespace infra
{
    namespace detail
    {
        template<class T>
        struct IntrusiveSetNode
            : IntrusiveBinarySearchTreeNode<T>
        {
            IntrusiveSetNode();
            IntrusiveSetNode(const IntrusiveSetNode& other);

            IntrusiveSetNode& operator=(const IntrusiveSetNode& other);

            enum class Colour
            {
                red,
                black
            };

            Colour colour = Colour::red;
        };
    }

    template<class T, class Compare = std::less<T>>
    class IntrusiveSet
        : public IntrusiveBinarySearchTree<T, Compare>
    {
    public:
        typedef typename IntrusiveBinarySearchTree<T, Compare>::value_type value_type;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::reference reference;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::const_reference const_reference;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::pointer pointer;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::const_pointer const_pointer;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::iterator iterator;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::const_iterator const_iterator;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::reverse_iterator reverse_iterator;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::const_reverse_iterator const_reverse_iterator;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::difference_type difference_type;
        typedef typename IntrusiveBinarySearchTree<T, Compare>::size_type size_type;

        typedef detail::IntrusiveSetNode<T> NodeType;
        typedef typename NodeType::Colour Colour;

    public:
        explicit IntrusiveSet(const Compare& compare = Compare());
        template<class InputIterator>
        IntrusiveSet(InputIterator first, InputIterator last, const Compare& comp = Compare());
        IntrusiveSet(const IntrusiveSet&) = delete;
        IntrusiveSet(IntrusiveSet&& other);
        IntrusiveSet& operator=(const IntrusiveSet&) = delete;
        IntrusiveSet& operator=(IntrusiveSet&& other);
        ~IntrusiveSet();

    public:
        // For test purposes
        bool invariant_holds() const;

    public:
        void insert(const_reference value);
        void erase(const_reference value);

        template<class InputIterator>
        void assign(InputIterator first, InputIterator last);

        void swap(IntrusiveSet& other);

    private:
        bool invariant_holds_for_each_path(const_reference node, bool mustBeBlack, std::size_t blackLevels) const;
    };

    template<class T, class Compare>
    void swap(IntrusiveSet<T, Compare>& node, IntrusiveSet<T, Compare>& uncle);

    ////    Implementation    ////

    template<class T, class Compare>
    IntrusiveSet<T, Compare>::IntrusiveSet(const Compare& compare)
        : IntrusiveBinarySearchTree<T, Compare>(compare)
    {}

    template<class T, class Compare>
    template<class InputIterator>
    IntrusiveSet<T, Compare>::IntrusiveSet(InputIterator first, InputIterator last, const Compare& compare)
        : IntrusiveBinarySearchTree<T, Compare>(compare)
    {
        assign(first, last);
    }

    template<class T, class Compare>
    IntrusiveSet<T, Compare>::IntrusiveSet(IntrusiveSet&& other)
    {
        *this = std::move(other);
    }

    template<class T, class Compare>
    IntrusiveSet<T, Compare>& IntrusiveSet<T, Compare>::operator=(IntrusiveSet&& other)
    {
        static_cast<IntrusiveBinarySearchTree<T, Compare>&>(*this) = std::move(static_cast<IntrusiveBinarySearchTree<T, Compare>&>(other));

        return *this;
    }

    template<class T, class Compare>
    IntrusiveSet<T, Compare>::~IntrusiveSet()
    {}

    template<class T, class Compare>
    bool IntrusiveSet<T, Compare>::invariant_holds() const
    {
        if (this->empty())
            return true;

        std::size_t blackLevels = 0;

        const NodeType* node = this->RootNode();

        while (node != nullptr)
        {
            if (node->colour != Colour::black && node->colour != Colour::red)
                return false;

            if (node->colour == Colour::black)
                ++blackLevels;

            if (!node->left)
                node = node->right;
            else
                node = node->left;
        }

        return (!this->RootNode() || invariant_holds_for_each_path(*this->RootNode(), true, blackLevels)) && IntrusiveBinarySearchTree<T, Compare>::invariant_holds();
    }

    template<class T, class Compare>
    bool IntrusiveSet<T, Compare>::invariant_holds_for_each_path(const_reference node, bool mustBeBlack, std::size_t blackLevels) const
    {
        if (mustBeBlack && node.colour != Colour::black)
            return false;

        if (node.colour == Colour::black)
            --blackLevels;

        if (!node.left && !node.right)
            return blackLevels == 0;

        mustBeBlack = node.colour == Colour::red;
        return ((!node.left && blackLevels == 0) || (node.left && invariant_holds_for_each_path(*node.left, mustBeBlack, blackLevels))) && ((!node.right && blackLevels == 0) || (node.right && invariant_holds_for_each_path(*node.right, mustBeBlack, blackLevels)));
    }

    template<class T, class Compare>
    void IntrusiveSet<T, Compare>::insert(const_reference value)
    {
        assert(!value.parent);
        assert(!value.left);
        assert(!value.right);

        T* node = const_cast<pointer>(&value);
        this->insert_in_tree(*node);
        node->colour = Colour::red;

        while (node != this->RootNode() && node->parent->colour == Colour::red)
        {
            if (node->parent == node->parent->parent->left)
            {
                T* uncle = node->parent->parent->right;
                if (uncle && uncle->colour == Colour::red)
                {
                    node->parent->colour = Colour::black;
                    uncle->colour = Colour::black;
                    node->parent->parent->colour = Colour::red;
                    node = node->parent->parent;
                }
                else
                {
                    if (node == node->parent->right)
                    {
                        node = node->parent;
                        this->left_rotate(*node);
                    }

                    node->parent->colour = Colour::black;
                    node->parent->parent->colour = Colour::red;
                    this->right_rotate(*node->parent->parent);
                }
            }
            else
            {
                T* uncle = node->parent->parent->left;
                if (uncle && uncle->colour == Colour::red)
                {
                    node->parent->colour = Colour::black;
                    uncle->colour = Colour::black;
                    node->parent->parent->colour = Colour::red;
                    node = node->parent->parent;
                }
                else
                {
                    if (node == node->parent->left)
                    {
                        node = node->parent;
                        this->right_rotate(*node);
                    }

                    node->parent->colour = Colour::black;
                    node->parent->parent->colour = Colour::red;
                    this->left_rotate(*node->parent->parent);
                }
            }
        }

        const_cast<T*>(this->RootNode())->colour = Colour::black;
    }

    template<class T, class Compare>
    void IntrusiveSet<T, Compare>::erase(const_reference value)
    {
        // See http://www.geeksforgeeks.org/red-black-tree-set-3-delete-2/ for the algorithm

        T* doubleBlackParent = nullptr;
        T** doubleBlack = nullptr;

        {
            // Step 1
            bool twoChildren = value.left && value.right;

            T* valueParent = value.parent;
            T* replacement;
            T* oldReplacementParent;
            T** oldReplacement;
            std::tie(replacement, oldReplacementParent, oldReplacement) = this->remove_from_tree(const_cast<T&>(value));

            if (oldReplacementParent != nullptr)
            {
                bool firstRed = replacement && replacement->colour == Colour::red;
                bool secondRed = oldReplacementParent == &value
                                     ? twoChildren ? *oldReplacement && (*oldReplacement)->colour == Colour::red : value.colour == Colour::red
                                     : *oldReplacement && (*oldReplacement)->colour == Colour::red;

                if (!firstRed && !secondRed)
                {
                    if (oldReplacementParent != &value)
                    {
                        doubleBlackParent = oldReplacementParent;
                        doubleBlack = oldReplacement;
                    }
                    else if (twoChildren)
                    {
                        doubleBlackParent = replacement;
                        doubleBlack = oldReplacement == &value.left ? &replacement->left : &replacement->right;
                    }
                    else
                    {
                        doubleBlackParent = valueParent;
                        doubleBlack = oldReplacement == &valueParent->left ? &valueParent->left : &valueParent->right;
                    }
                }
                else
                {
                    // Step 2, the simple case: either u or v is red (colour is copied below,
                    // a node being deleted with only one child is always black, since as red it cannot have a red child (two consecutive reds),
                    // nor a black child (one leaf and one black child is a violation of balance)
                }
            }

            if (replacement != nullptr)
            {
                if (oldReplacement != nullptr && *oldReplacement != nullptr)
                    (*oldReplacement)->colour = replacement->colour;
                replacement->colour = value.colour;
            }
        }

        while (doubleBlackParent != nullptr)
        {
            T* doubleBlackSibling = &doubleBlackParent->left == doubleBlack ? doubleBlackParent->right : doubleBlackParent->left;

            if (doubleBlackSibling->colour == Colour::black && ((doubleBlackSibling->left && doubleBlackSibling->left->colour == Colour::red) || (doubleBlackSibling->right && doubleBlackSibling->right->colour == Colour::red)))
            {
                T** redNephew = (doubleBlackSibling->left && doubleBlackSibling->left->colour == Colour::red) ? &doubleBlackSibling->left : &doubleBlackSibling->right;
                (*redNephew)->colour = Colour::black;

                if (doubleBlackSibling == doubleBlackParent->left && redNephew == &doubleBlackSibling->left)
                {
                    // Step 3.2 a i
                    this->right_rotate(*doubleBlackParent);
                    doubleBlackSibling->colour = doubleBlackParent->colour;
                    doubleBlackParent->colour = Colour::black;
                }
                else if (doubleBlackSibling == doubleBlackParent->left && redNephew == &doubleBlackSibling->right)
                {
                    // Step 3.2 a ii
                    (*redNephew)->colour = doubleBlackParent->colour;
                    doubleBlackParent->colour = Colour::black;
                    doubleBlackSibling->colour = Colour::black;

                    this->left_rotate(*doubleBlackSibling);
                    this->right_rotate(*doubleBlackParent);
                }
                else if (doubleBlackSibling == doubleBlackParent->right && redNephew == &doubleBlackSibling->right)
                {
                    // Step 3.2 a iii
                    this->left_rotate(*doubleBlackParent);
                    doubleBlackSibling->colour = doubleBlackParent->colour;
                    doubleBlackParent->colour = Colour::black;
                }
                else if (doubleBlackSibling == doubleBlackParent->right && redNephew == &doubleBlackSibling->left)
                {
                    // Step 3.2 a iv
                    (*redNephew)->colour = doubleBlackParent->colour;
                    doubleBlackParent->colour = Colour::black;
                    doubleBlackSibling->colour = Colour::black;

                    this->right_rotate(*doubleBlackSibling);
                    this->left_rotate(*doubleBlackParent);
                }
                else
                    abort();

                doubleBlackParent = nullptr;
            }
            else if (doubleBlackSibling->colour == Colour::black && (!doubleBlackSibling->left || doubleBlackSibling->left->colour == Colour::black) && (!doubleBlackSibling->right || doubleBlackSibling->right->colour == Colour::black))
            {
                // Step 3.2 b
                doubleBlackSibling->colour = Colour::red;
                if (doubleBlackParent->colour == Colour::red)
                {
                    doubleBlackParent->colour = Colour::black;
                    doubleBlackParent = nullptr;
                }
                else
                {
                    if (doubleBlackParent->parent != nullptr)
                    {
                        if (doubleBlackParent->parent->left == doubleBlackParent)
                            doubleBlack = &doubleBlackParent->parent->left;
                        else
                            doubleBlack = &doubleBlackParent->parent->right;
                    }

                    doubleBlackParent = doubleBlackParent->parent;
                }
            }
            else if (doubleBlackSibling->colour == Colour::red)
            {
                doubleBlackSibling->colour = Colour::black;
                doubleBlackParent->colour = Colour::red;

                if (doubleBlackSibling == doubleBlackParent->left)
                {
                    // Step 3.2 c i
                    this->right_rotate(*doubleBlackParent);
                    doubleBlack = &doubleBlackParent->right;
                }
                else
                {
                    // Step 3.2 c ii
                    this->left_rotate(*doubleBlackParent);
                    doubleBlack = &doubleBlackParent->left;
                }
            }
            else
                abort();
        }

        if (this->RootNode() != nullptr)
            const_cast<T*>(this->RootNode())->colour = Colour::black;
    }

    template<class T, class Compare>
    template<class InputIterator>
    void IntrusiveSet<T, Compare>::assign(InputIterator first, InputIterator last)
    {
        this->clear();
        for (; first != last; ++first)
            insert(*first);
    }

    template<class T, class Compare>
    void IntrusiveSet<T, Compare>::swap(IntrusiveSet& other)
    {
        IntrusiveBinarySearchTree<T, Compare>::swap(other);
    }

    template<class T, class Compare>
    void swap(IntrusiveSet<T, Compare>& node, IntrusiveSet<T, Compare>& uncle)
    {
        node.swap(uncle);
    }

    namespace detail
    {
        template<class T>
        IntrusiveSetNode<T>::IntrusiveSetNode()
        {}

        template<class T>
        IntrusiveSetNode<T>::IntrusiveSetNode(const IntrusiveSetNode& other)
        {}

        template<class T>
        IntrusiveSetNode<T>& IntrusiveSetNode<T>::operator=(const IntrusiveSetNode& other)
        {
            return *this;
        }
    }

}

#endif
