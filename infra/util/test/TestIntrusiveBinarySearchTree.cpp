#include "gtest/gtest.h"
#include "infra/util/IntrusiveBinarySearchTree.hpp"
#include <sstream>
#include <string>

namespace
{

    class TestNode
        : public infra::IntrusiveBinarySearchTree<TestNode>::NodeType
    {
    public:
        TestNode(int value)
            : value(value)
        {}

        bool operator==(const TestNode& other) const
        {
            return value == other.value;
        }

        bool operator<(const TestNode& other) const
        {
            return value < other.value;
        }

    public:
        int value;
    };

}

class IntrusiveBinarySearchTreeTest
    : public testing::Test
{
public:
    ~IntrusiveBinarySearchTreeTest()
    {
        EXPECT_TRUE(tree.invariant_holds());
    }

    std::string Representation(const infra::IntrusiveBinarySearchTree<TestNode>& tree)
    {
        std::ostringstream os;

        if (!tree.empty())
            AddRepresentation(tree.root(), os);

        return os.str();
    }

    void AddRepresentation(const TestNode& node, std::ostream& os)
    {
        os << node.value;

        if (node.left || node.right)
        {
            os << "(";
            if (node.left)
                AddRepresentation(*node.left, os);
            os << ",";
            if (node.right)
                AddRepresentation(*node.right, os);
            os << ")";
        }
    }

public:
    TestNode node0 = 0;
    TestNode node1 = 1;
    TestNode node2 = 2;
    TestNode node3 = 3;
    TestNode node4 = 4;
    TestNode node5 = 5;

    infra::IntrusiveBinarySearchTree<TestNode> tree;
};

TEST_F(IntrusiveBinarySearchTreeTest, ConstructEmptyTree)
{
    EXPECT_TRUE(tree.invariant_holds());
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_EQ(tree.begin(), tree.end());
    EXPECT_EQ(tree.rbegin(), tree.rend());
    EXPECT_EQ(tree.cbegin(), tree.cend());
    EXPECT_EQ(tree.crbegin(), tree.crend());
    EXPECT_EQ(tree.begin(), tree.cend());
    EXPECT_EQ(tree.rbegin(), tree.crend());
}

TEST_F(IntrusiveBinarySearchTreeTest, MoveConstructEmptyTree)
{
    infra::IntrusiveBinarySearchTree<TestNode> moved(std::move(tree));

    EXPECT_TRUE(moved.invariant_holds());
    EXPECT_TRUE(moved.empty());
    EXPECT_EQ(0, moved.size());
    EXPECT_EQ(moved.begin(), moved.end());
    EXPECT_EQ(moved.rbegin(), moved.rend());
    EXPECT_EQ(moved.cbegin(), moved.cend());
    EXPECT_EQ(moved.crbegin(), moved.crend());
    EXPECT_EQ(moved.begin(), moved.cend());
    EXPECT_EQ(moved.rbegin(), moved.crend());

    EXPECT_TRUE(tree.invariant_holds());
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_EQ(tree.begin(), tree.end());
    EXPECT_EQ(tree.rbegin(), tree.rend());
    EXPECT_EQ(tree.cbegin(), tree.cend());
    EXPECT_EQ(tree.crbegin(), tree.crend());
    EXPECT_EQ(tree.begin(), tree.cend());
    EXPECT_EQ(tree.rbegin(), tree.crend());
}

TEST_F(IntrusiveBinarySearchTreeTest, MoveAssignEmptyTree)
{
    infra::IntrusiveBinarySearchTree<TestNode> moved;
    moved = std::move(tree);

    EXPECT_TRUE(moved.invariant_holds());
    EXPECT_TRUE(moved.empty());
    EXPECT_EQ(0, moved.size());
    EXPECT_EQ(moved.begin(), moved.end());
    EXPECT_EQ(moved.rbegin(), moved.rend());
    EXPECT_EQ(moved.cbegin(), moved.cend());
    EXPECT_EQ(moved.crbegin(), moved.crend());
    EXPECT_EQ(moved.begin(), moved.cend());
    EXPECT_EQ(moved.rbegin(), moved.crend());

    EXPECT_TRUE(tree.invariant_holds());
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(0, tree.size());
    EXPECT_EQ(tree.begin(), tree.end());
    EXPECT_EQ(tree.rbegin(), tree.rend());
    EXPECT_EQ(tree.cbegin(), tree.cend());
    EXPECT_EQ(tree.crbegin(), tree.crend());
    EXPECT_EQ(tree.begin(), tree.cend());
    EXPECT_EQ(tree.rbegin(), tree.crend());
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertNode)
{
    tree.insert_in_tree(node0);

    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(1, tree.size());
    EXPECT_EQ("0", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertSecondNodeLeft)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);

    EXPECT_EQ("1(0,)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertSecondNodeRight)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node2);

    EXPECT_EQ("1(,2)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertThirdNodeLeft)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node0);

    EXPECT_EQ("1(0,2)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertThirdNodeRight)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    EXPECT_EQ("1(0,2)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertThirdNodeLeftInBetween)
{
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node1);

    EXPECT_EQ("2(0(,1),)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, InsertThirdNodeRightInBetween)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);

    EXPECT_EQ("0(,2(1,))", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveSingleNode)
{
    tree.insert_in_tree(node0);
    ASSERT_EQ("0", Representation(tree));

    EXPECT_EQ(std::make_tuple(nullptr, nullptr, nullptr), tree.remove_from_tree(node0));
    EXPECT_EQ("", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveRootNodeWithLeftChild)
{
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);
    ASSERT_EQ("2(1,)", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node1, &node2, &node2.left), tree.remove_from_tree(node2));
    EXPECT_EQ("1", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveRootNodeWithRightChild)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node1);
    ASSERT_EQ("0(,1)", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node1, &node0, &node0.right), tree.remove_from_tree(node0));
    EXPECT_EQ("1", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveRootNodeWithTwoChildren)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    ASSERT_EQ("1(0,2)", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node2, &node1, &node1.right), tree.remove_from_tree(node1));
    EXPECT_EQ("2(0,)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveRootNodeWithTwoChildrenAndGrandChildren)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node2);
    ASSERT_EQ("1(0,3(2,))", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node2, &node3, &node3.left), tree.remove_from_tree(node1));
    EXPECT_EQ("2(0,3)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootLeftLeafNode)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    ASSERT_EQ("1(0,)", Representation(tree));

    EXPECT_EQ(std::make_tuple(nullptr, &node0, &node1.left), tree.remove_from_tree(node0));
    EXPECT_EQ("1", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootRightLeafNode)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node2);
    ASSERT_EQ("1(,2)", Representation(tree));

    EXPECT_EQ(std::make_tuple(nullptr, &node2, &node1.right), tree.remove_from_tree(node2));
    EXPECT_EQ("1", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootLeftChildWithLeftGrandChild)
{
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    ASSERT_EQ("2(1(0,),)", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node0, &node1, &node1.left), tree.remove_from_tree(node1));
    EXPECT_EQ("2(0,)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootLeftChildWithRightGrandChild)
{
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node1);
    ASSERT_EQ("2(0(,1),)", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node1, &node0, &node0.right), tree.remove_from_tree(node0));
    EXPECT_EQ("2(1,)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootRightChildWithLeftGrandChild)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);
    ASSERT_EQ("0(,2(1,))", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node1, &node2, &node2.left), tree.remove_from_tree(node2));
    EXPECT_EQ("0(,1)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootRightChildWithRightGrandChild)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node2);
    ASSERT_EQ("0(,1(,2))", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node2, &node1, &node1.right), tree.remove_from_tree(node1));
    EXPECT_EQ("0(,2)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNodeWithTwoChildren)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node4);
    ASSERT_EQ("1(0,3(2,4))", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node4, &node3, &node3.right), tree.remove_from_tree(node3));
    EXPECT_EQ("1(0,4(2,))", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNodeWithTwoChildrenAndGrandChildren)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node2);
    ASSERT_EQ("1(0,3(2,))", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node2, &node3, &node3.left), tree.remove_from_tree(node1));
    EXPECT_EQ("2(0,3)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNodeWithTwoChildrenAndMoreGrandChildren)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node4);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node3);
    ASSERT_EQ("1(0,4(2(,3),))", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node2, &node4, &node4.left), tree.remove_from_tree(node1));
    EXPECT_EQ("2(0,4(3,))", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveNonRootNodeWithTwoChildrenAndMoreGrandChildren)
{
    tree.insert_in_tree(node5);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node4);
    ASSERT_EQ("5(1(0,3(2,4)),)", Representation(tree));

    EXPECT_EQ(std::make_tuple(&node2, &node3, &node3.left), tree.remove_from_tree(node1));
    EXPECT_EQ("5(2(0,3(,4)),)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveFrontWithReplacementNotBeingTheNewFront)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);
    ASSERT_EQ("0(,2(1,))", Representation(tree));

    tree.remove_from_tree(node0);
}

TEST_F(IntrusiveBinarySearchTreeTest, RemoveBackWithReplacementNotBeingTheNewBack)
{
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node1);
    ASSERT_EQ("2(0(,1),)", Representation(tree));

    tree.remove_from_tree(node2);
}

TEST_F(IntrusiveBinarySearchTreeTest, Accessors)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    EXPECT_EQ(&node1, &tree.root());
    EXPECT_EQ(&node0, &tree.front());
    EXPECT_EQ(&node2, &tree.back());
}

TEST_F(IntrusiveBinarySearchTreeTest, SwapEmptyWithEmpty)
{
    infra::IntrusiveBinarySearchTree<TestNode> other;

    swap(tree, other);

    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(other.empty());
    EXPECT_TRUE(other.invariant_holds());
}

TEST_F(IntrusiveBinarySearchTreeTest, SwapFilledWithEmpty)
{
    infra::IntrusiveBinarySearchTree<TestNode> other;
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    swap(tree, other);

    EXPECT_TRUE(tree.empty());
    EXPECT_FALSE(other.empty());
    EXPECT_EQ(3, other.size());
    EXPECT_EQ(&node1, &other.root());
    EXPECT_TRUE(other.invariant_holds());
}

TEST_F(IntrusiveBinarySearchTreeTest, SwapEmptyWithFilled)
{
    infra::IntrusiveBinarySearchTree<TestNode> other;
    other.insert_in_tree(node1);
    other.insert_in_tree(node0);
    other.insert_in_tree(node2);

    swap(tree, other);

    EXPECT_FALSE(tree.empty());
    EXPECT_TRUE(other.empty());
    EXPECT_EQ(3, tree.size());
    EXPECT_EQ(&node1, &tree.root());
    EXPECT_TRUE(other.invariant_holds());
}

TEST_F(IntrusiveBinarySearchTreeTest, SwapFilledWithFilled)
{
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node4);
    infra::IntrusiveBinarySearchTree<TestNode> other;
    other.insert_in_tree(node1);
    other.insert_in_tree(node0);
    other.insert_in_tree(node2);

    swap(tree, other);

    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(3, tree.size());
    EXPECT_EQ(&node1, &tree.root());
    EXPECT_FALSE(other.empty());
    EXPECT_EQ(2, other.size());
    EXPECT_EQ(&node3, &other.root());
    EXPECT_TRUE(other.invariant_holds());
}

TEST_F(IntrusiveBinarySearchTreeTest, FindRootNode)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    EXPECT_EQ(0, tree.find(node0)->value);
}

TEST_F(IntrusiveBinarySearchTreeTest, FindLeftNode)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    EXPECT_EQ(1, tree.find(node1)->value);
}

TEST_F(IntrusiveBinarySearchTreeTest, FindRightNode)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    EXPECT_EQ(2, tree.find(node2)->value);
}

TEST_F(IntrusiveBinarySearchTreeTest, DontFindNode)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);

    EXPECT_EQ(tree.end(), tree.find(node3));
}

TEST_F(IntrusiveBinarySearchTreeTest, RotateRootLeft)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node4);
    ASSERT_EQ("1(0,3(2,4))", Representation(tree));

    tree.left_rotate(node1);
    EXPECT_EQ("3(1(0,2),4)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RotateLeftChildLeft)
{
    tree.insert_in_tree(node5);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node4);
    ASSERT_EQ("5(1(0,3(2,4)),)", Representation(tree));

    tree.left_rotate(node1);
    EXPECT_EQ("5(3(1(0,2),4),)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RotateRightChildLeft)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node4);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node5);
    ASSERT_EQ("0(,2(1,4(3,5)))", Representation(tree));

    tree.left_rotate(node2);
    EXPECT_EQ("0(,4(2(1,3),5))", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RotateRootRight)
{
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node4);
    ASSERT_EQ("3(1(0,2),4)", Representation(tree));

    tree.right_rotate(node3);
    EXPECT_EQ("1(0,3(2,4))", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RotateLeftChildRight)
{
    tree.insert_in_tree(node5);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node4);
    EXPECT_EQ("5(3(1(0,2),4),)", Representation(tree));

    tree.right_rotate(node3);
    ASSERT_EQ("5(1(0,3(2,4)),)", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, RotateRightChildRight)
{
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node4);
    tree.insert_in_tree(node2);
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node3);
    tree.insert_in_tree(node5);
    ASSERT_EQ("0(,4(2(1,3),5))", Representation(tree));

    tree.right_rotate(node4);
    EXPECT_EQ("0(,2(1,4(3,5)))", Representation(tree));
}

TEST_F(IntrusiveBinarySearchTreeTest, Sibling)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    ASSERT_EQ("1(0,2)", Representation(tree));

    EXPECT_EQ(&node0, tree.sibling(node2));
    EXPECT_EQ(&node2, tree.sibling(node0));
}

TEST_F(IntrusiveBinarySearchTreeTest, ParentChildReference)
{
    tree.insert_in_tree(node1);
    tree.insert_in_tree(node0);
    tree.insert_in_tree(node2);
    ASSERT_EQ("1(0,2)", Representation(tree));

    EXPECT_EQ(&node1.left, &tree.parent_child_reference(node1, node0));
    EXPECT_EQ(&node1.right, &tree.parent_child_reference(node1, node2));
}

TEST_F(IntrusiveBinarySearchTreeTest, TestManyInsertionsAndDeletions)
{
    for (int size = 1; size != 20; ++size)
    {
        for (int step = 3; step != 6; ++step)
        {
            std::vector<TestNode> elements;
            int n = 0;
            std::generate_n(std::back_inserter(elements), size, [&n]() { return n++; });
            std::vector<std::size_t> indices;
            n = 0;
            std::generate_n(std::back_inserter(indices), size, [&n]() { return n++; });

            infra::IntrusiveBinarySearchTree<TestNode> tree;
            int current = 0;
            while (!indices.empty())
            {
                current = (current + step) % indices.size();
                tree.insert_in_tree(elements[indices[current]]);
                EXPECT_TRUE(tree.invariant_holds());
                indices.erase(indices.begin() + current);
            }

            while (!tree.empty())
            {
                current = (current + step) % tree.size();
                tree.remove_from_tree(*std::next(tree.begin(), current));
                EXPECT_TRUE(tree.invariant_holds());
            }
        }
    }
}
