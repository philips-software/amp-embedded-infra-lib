#include "infra/util/BoundedList.hpp"
#include "infra/util/test_helper/MonitoredConstructionObject.hpp"
#include "infra/util/test_helper/MoveConstructible.hpp"
#include "gtest/gtest.h"
#include <functional>

TEST(BoundedListTest, TestConstructedEmpty)
{
    infra::BoundedList<int>::WithMaxSize<5> list;

    EXPECT_TRUE(list.empty());
    EXPECT_FALSE(list.full());
    EXPECT_EQ(0, list.size());
    EXPECT_EQ(5, list.max_size());
}

TEST(BoundedListTest, TestConstructionWith2Elements)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(2), 4);

    EXPECT_FALSE(list.empty());
    EXPECT_FALSE(list.full());
    EXPECT_EQ(2, list.size());

    EXPECT_EQ(4, *list.begin());
    EXPECT_EQ(4, *std::next(list.begin()));
}

TEST(BoundedListTest, TestConstructionWithRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(3, list.size());

    EXPECT_EQ(0, *list.begin());
    EXPECT_EQ(1, *std::next(list.begin()));
    EXPECT_EQ(2, *std::next(list.begin(), 2));
}

TEST(BoundedListTest, TestConstructionWithInitializerList)
{
    infra::BoundedList<int>::WithMaxSize<5> list({ 0, 1, 2 });

    EXPECT_EQ(3, list.size());

    EXPECT_EQ(0, *list.begin());
    EXPECT_EQ(1, *std::next(list.begin()));
    EXPECT_EQ(2, *std::next(list.begin(), 2));
}

TEST(BoundedListTest, TestCopyConstruction)
{
    infra::BoundedList<int>::WithMaxSize<5> original(std::size_t(2), 4);
    infra::BoundedList<int>::WithMaxSize<5> copy(original);

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(4, copy.front());
}

TEST(BoundedListTest, TestMoveConstruction)
{
    infra::BoundedList<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_front(2);
    infra::BoundedList<infra::MoveConstructible>::WithMaxSize<5> copy(std::move(original));

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy.front().x);
}

TEST(BoundedListTest, TestAssignment)
{
    infra::BoundedList<int>::WithMaxSize<5> original(std::size_t(2), 4);
    infra::BoundedList<int>::WithMaxSize<5> copy;
    copy = original;

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(4, copy.front());
}

TEST(BoundedListTest, TestSelfAssignment)
{
    infra::BoundedList<int>::WithMaxSize<5> original(std::size_t(2), 4);
    original = original;

    EXPECT_EQ(2, original.size());
    EXPECT_EQ(4, original.front());
}

TEST(BoundedListTest, TestMove)
{
    infra::BoundedList<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_front(2);
    infra::BoundedList<infra::MoveConstructible>::WithMaxSize<5> copy;
    copy = std::move(original);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy.front().x);
}

TEST(BoundedListTest, TestBeginAndEnd)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(list.end(), std::next(list.begin(), 3));
    EXPECT_EQ(list.cbegin(), list.begin());
    EXPECT_EQ(list.cend(), list.end());

    EXPECT_EQ(0, *list.begin());
}

TEST(BoundedListTest, TestFull)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(5), 4);

    EXPECT_TRUE(list.full());
}

TEST(BoundedListTest, TestFront)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(0, list.front());
    EXPECT_EQ(0, static_cast<const infra::BoundedList<int>&>(list).front());
}

TEST(BoundedListTest, TestBack)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(2, list.back());
    EXPECT_EQ(2, static_cast<const infra::BoundedList<int>&>(list).back());
}

TEST(BoundedListTest, TestAssignRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list(range, range + 3);

    int otherRange[2] = { 4, 5 };
    list.assign(otherRange, otherRange + 2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(4, list.front());
    EXPECT_EQ(5, *std::next(list.begin()));
}

TEST(BoundedListTest, TestAssignN)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list(range, range + 3);

    list.assign(std::size_t(2), 4);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(4, list.front());
    EXPECT_EQ(4, *std::next(list.begin()));
}

TEST(BoundedListTest, TestMoveFromRange)
{
    infra::BoundedList<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_front(2);
    infra::BoundedList<infra::MoveConstructible>::WithMaxSize<5> copy;
    copy.move_from_range(original.begin(), original.end());

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy.front().x);
}

TEST(BoundedListTest, TestPushFront)
{
    infra::BoundedList<int>::WithMaxSize<5> list;
    int i(1);
    list.push_front(i);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.front());

    ++i;
    list.push_front(i);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, list.front());
}

TEST(BoundedListTest, TestPushFrontRvalue)
{
    infra::BoundedList<int>::WithMaxSize<5> list;
    list.push_front(1);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.front());

    list.push_front(2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, list.front());
}

TEST(BoundedListTest, TestEmplaceFront)
{
    infra::BoundedList<int>::WithMaxSize<5> list;
    list.emplace_front(1);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.front());

    list.emplace_front(2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, list.front());
}

TEST(BoundedListTest, TestPopFront)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.pop_front();

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(4, list.front());

    list.pop_front();

    EXPECT_TRUE(list.empty());
}

TEST(BoundedListTest, TestPushBack)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.push_back(1);

    EXPECT_EQ(3, list.size());
    EXPECT_EQ(1, list.back());
}

TEST(BoundedListTest, TestEmplaceBack)
{
    infra::BoundedList<int>::WithMaxSize<5> list;
    list.emplace_back(1);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.back());

    list.emplace_back(2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, list.back());
}

TEST(BoundedListTest, TestPopBack)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.pop_front();

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(4, list.back());
}

TEST(BoundedListTest, TestSwap)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5 };
    infra::BoundedList<int>::WithMaxSize<5> list2(range2, range2 + 3);

    swap(list1, list2);

    infra::BoundedList<int>::WithMaxSize<5> expectedList1(range2, range2 + 3);
    infra::BoundedList<int>::WithMaxSize<5> expectedList2(range1, range1 + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(BoundedListTest, TestSwapDifferentSizes)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[2] = { 3, 4 };
    infra::BoundedList<int>::WithMaxSize<5> list2(range2, range2 + 2);

    swap(list1, list2);

    infra::BoundedList<int>::WithMaxSize<5> expectedList1(range2, range2 + 2);
    infra::BoundedList<int>::WithMaxSize<5> expectedList2(range1, range1 + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(BoundedListTest, TestClear)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.clear();

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());

    infra::BoundedList<int>::WithMaxSize<5> expectedList;
    EXPECT_EQ(expectedList, list);
}

TEST(BoundedListTest, TestInsert)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(1), 4);
    int i(2);
    list.insert(list.begin(), i);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, list.front());

    ++i;
    list.insert(std::next(list.begin()), i);

    EXPECT_EQ(3, list.size());
    EXPECT_EQ(3, *std::next(list.begin()));

    ++i;
    list.insert(list.end(), i);

    EXPECT_EQ(4, list.size());
    EXPECT_EQ(4, list.back());
}

TEST(BoundedListTest, TestInsertRvalue)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(1), 4);
    list.insert(list.begin(), 2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, list.front());

    list.insert(std::next(list.begin()), 3);

    EXPECT_EQ(3, list.size());
    EXPECT_EQ(3, *std::next(list.begin()));

    list.insert(list.end(), 4);

    EXPECT_EQ(4, list.size());
    EXPECT_EQ(4, list.back());
}

TEST(BoundedListTest, TestErase)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(3), 4);
    list.erase(std::next(list.begin()));
    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, std::distance(list.begin(), list.end()));

    list.erase(std::prev(list.end()));
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));

    list.erase(list.begin());
    EXPECT_EQ(0, list.size());
    EXPECT_EQ(0, std::distance(list.begin(), list.end()));
}

TEST(BoundedListTest, TestEraseAllAfter)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(3), 4);

    list.erase_all_after(list.begin());
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));
}

TEST(BoundedListTest, TestEraseAllAfterNothing)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    list.erase_all_after(list.begin());
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));
}

TEST(BoundedListTest, TestRemove)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(3), 4);
    list.remove(*std::next(list.begin()));
    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, std::distance(list.begin(), list.end()));

    list.remove(list.back());
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));

    list.remove(list.front());
    EXPECT_EQ(0, list.size());
    EXPECT_EQ(0, std::distance(list.begin(), list.end()));
}

class BoundedListConstructionTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::ConstructionMonitorMock> constructionMonitor;

    class MonitoredConstructionInt
        : infra::MonitoredConstructionObject
    {
    public:
        MonitoredConstructionInt(infra::ConstructionMonitorMock& constructionMonitor)
            : MonitoredConstructionObject(constructionMonitor)
        {}
    };
};

TEST_F(BoundedListConstructionTest, TestEraseDestruction)
{
    EXPECT_CALL(constructionMonitor, Construct(testing::_));

    infra::BoundedList<MonitoredConstructionInt>::WithMaxSize<1> monitoredList;
    monitoredList.emplace_front(constructionMonitor);
    EXPECT_EQ(1, monitoredList.size());

    EXPECT_CALL(constructionMonitor, Destruct(testing::_));
    monitoredList.erase(monitoredList.begin());
    EXPECT_EQ(0, monitoredList.size());
}

TEST_F(BoundedListConstructionTest, TestEraseAllAfterDestruction)
{
    infra::BoundedList<MonitoredConstructionInt>::WithMaxSize<5> monitoredList;

    EXPECT_CALL(constructionMonitor, Construct(testing::_)).Times(3);

    monitoredList.emplace_front(constructionMonitor);
    monitoredList.emplace_front(constructionMonitor);
    monitoredList.emplace_front(constructionMonitor);
    EXPECT_EQ(3, monitoredList.size());

    EXPECT_CALL(constructionMonitor, Destruct(testing::_)).Times(2);
    monitoredList.erase_all_after(monitoredList.begin());
    EXPECT_EQ(1, monitoredList.size());

    EXPECT_CALL(constructionMonitor, Destruct(testing::_)).Times(1);
}

TEST(BoundedListTest, IteratorCopyConstruct)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedList<int>::iterator i(list.begin());
    EXPECT_EQ(4, *i);
}

TEST(BoundedListTest, IteratorArrow)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    EXPECT_EQ(4, *(list.begin().operator->()));
}

TEST(BoundedListTest, IteratorPostInc)
{
    infra::BoundedList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedList<int>::iterator i(list.begin());
    EXPECT_EQ(4, *i++);
}

TEST(BoundedListTest, TestEquals)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5 };
    infra::BoundedList<int>::WithMaxSize<5> list2(range2, range2 + 3);
    int range3[2] = { 0, 1 };
    infra::BoundedList<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 == list1);
    EXPECT_FALSE(list1 == list2);
    EXPECT_FALSE(list1 == deque3);
    EXPECT_FALSE(deque3 == list1);
}

TEST(BoundedListTest, TestUnequals)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 != list1);
}

TEST(BoundedListTest, TestLessThan)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[3] = { 0, 4, 5 };
    infra::BoundedList<int>::WithMaxSize<5> list2(range2, range2 + 3);
    int range3[2] = { 0, 1 };
    infra::BoundedList<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 < list2);
    EXPECT_FALSE(list1 < deque3);
    EXPECT_FALSE(list1 < list1);
}

TEST(BoundedListTest, TestGreaterThan)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 > list1);
}

TEST(BoundedListTest, TestLessThanOrEqual)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 <= list1);
}

TEST(BoundedListTest, TestGreaterThanOrEqual)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 >= list1);
}
