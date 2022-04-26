#include "gmock/gmock.h"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/SharedPtr.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MonitoredConstructionObject.hpp"

namespace
{
    class MySharedObjectBase
    {
    protected:
        MySharedObjectBase() = default;
        MySharedObjectBase(const MySharedObjectBase& other) = delete;
        MySharedObjectBase& operator=(const MySharedObjectBase& other) = delete;
        ~MySharedObjectBase() = default;
    };

    class MySharedObject
        : public MySharedObjectBase
        , public infra::MonitoredConstructionObject
    {
    public:
        using infra::MonitoredConstructionObject::MonitoredConstructionObject;

        int Value() const
        {
            return 5;
        }
    };

    using AllocatorMySharedObject = infra::SharedObjectAllocator<MySharedObject, void(infra::ConstructionMonitorMock&)>;

    class OtherBase
    {
    public:
        int x;
    };

    class MultiInheritedClass
        : public OtherBase
        , public MySharedObject
    {
    public:
        MultiInheritedClass(infra::ConstructionMonitorMock& mock)
            : MySharedObject(mock)
        {}
    };
}

class SharedPtrTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::ConstructionMonitorMock> objectConstructionMock;
    AllocatorMySharedObject::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<2> allocator;
};

TEST_F(SharedPtrTest, construct_empty_SharedPtr)
{
    infra::SharedPtr<MySharedObject> object;
    EXPECT_FALSE(static_cast<bool>(object));
}

TEST_F(SharedPtrTest, convert_nullptr_to_empty_SharedPtr)
{
    infra::SharedPtr<MySharedObject> object(nullptr);
    EXPECT_FALSE(static_cast<bool>(object));

    infra::SharedPtr<MySharedObject> object2 = nullptr;
    EXPECT_FALSE(static_cast<bool>(object2));
}

TEST_F(SharedPtrTest, share_one_object_by_copy_construction)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    {
        infra::SharedPtr<MySharedObject> object2(object);
        EXPECT_TRUE(static_cast<bool>(object2));
    }

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, share_one_object_by_assignment)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    {
        infra::SharedPtr<MySharedObject> object2;
        object2 = object;
        EXPECT_TRUE(static_cast<bool>(object2));
    }

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, reset_SharedPtr_by_assigning_nullptr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    object = nullptr;
    testing::Mock::VerifyAndClearExpectations(&objectConstructionMock);
}

TEST_F(SharedPtrTest, move_construct_SharedPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    infra::SharedPtr<MySharedObject> object2(std::move(object));
    EXPECT_FALSE(static_cast<bool>(object));

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, move_copy_SharedPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    infra::SharedPtr<MySharedObject> object2;
    object2 = std::move(object);
    EXPECT_FALSE(static_cast<bool>(object));

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, self_assign_SharedPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    object = object;
    EXPECT_TRUE(static_cast<bool>(object));

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, convert_SharedPtr_to_and_from_const)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    infra::SharedPtr<const MySharedObject> constObject(object);
    EXPECT_TRUE(static_cast<bool>(constObject));
    infra::SharedPtr<const MySharedObject> movedConstObject(std::move(object));
    EXPECT_FALSE(static_cast<bool>(object));
    EXPECT_TRUE(static_cast<bool>(movedConstObject));

    infra::SharedPtr<MySharedObject> nonConstObject(infra::ConstPointerCast(constObject));
    EXPECT_TRUE(static_cast<bool>(nonConstObject));
    infra::SharedPtr<MySharedObject> movedNonConstObject(infra::ConstPointerCast(std::move(constObject)));
    EXPECT_FALSE(static_cast<bool>(constObject));
    EXPECT_TRUE(static_cast<bool>(movedNonConstObject));

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, ConstPointerCast_and_SharedPointerCast_respect_nullptr)
{
    infra::SharedPtr<const MySharedObjectBase> constObject;

    EXPECT_EQ(nullptr, infra::ConstPointerCast(constObject));
    EXPECT_EQ(nullptr, infra::ConstPointerCast(std::move(constObject)));

    EXPECT_EQ(nullptr, infra::StaticPointerCast<const MySharedObject>(constObject));
    EXPECT_EQ(nullptr, infra::StaticPointerCast<const MySharedObject>(std::move(constObject)));
}

TEST_F(SharedPtrTest, convert_SharedPtr_to_and_from_base)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    infra::SharedPtr<MySharedObjectBase> baseObject(object);
    EXPECT_TRUE(static_cast<bool>(baseObject));
    infra::SharedPtr<MySharedObjectBase> movedBaseObject(std::move(object));
    EXPECT_FALSE(static_cast<bool>(object));
    EXPECT_TRUE(static_cast<bool>(movedBaseObject));

    infra::SharedPtr<MySharedObject> derivedObject(infra::StaticPointerCast<MySharedObject>(baseObject));
    EXPECT_TRUE(static_cast<bool>(derivedObject));
    infra::SharedPtr<MySharedObject> movedDerivedObject(infra::StaticPointerCast<MySharedObject>(std::move(baseObject)));
    EXPECT_FALSE(static_cast<bool>(baseObject));
    EXPECT_TRUE(static_cast<bool>(movedDerivedObject));

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, dereference_SharedPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    const infra::SharedPtr<const MySharedObject> object = allocator.Allocate(objectConstructionMock);

    EXPECT_EQ(5, object->Value());
    EXPECT_EQ(5, (*object).Value());

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, test_equality)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).Times(2);
    infra::SharedPtr<MySharedObject> object1 = allocator.Allocate(objectConstructionMock);
    infra::SharedPtr<MySharedObject> object2 = allocator.Allocate(objectConstructionMock);
    infra::SharedPtr<MySharedObject> objectEmpty;

    infra::SharedPtr<MySharedObject> object1Same = object1;

    EXPECT_TRUE(object1 == object1Same);
    EXPECT_FALSE(object1 != object1Same);
    EXPECT_FALSE(object1 == object2);
    EXPECT_TRUE(object1 != object2);
    EXPECT_FALSE(object1 == objectEmpty);
    EXPECT_TRUE(object1 != objectEmpty);

    EXPECT_FALSE(object1 == nullptr);
    EXPECT_FALSE(nullptr == object1);
    EXPECT_TRUE(object1 != nullptr);
    EXPECT_TRUE(nullptr != object1);

    EXPECT_TRUE(objectEmpty == nullptr);
    EXPECT_TRUE(nullptr == objectEmpty);
    EXPECT_FALSE(objectEmpty != nullptr);
    EXPECT_FALSE(nullptr != objectEmpty);

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_)).Times(2);
}

TEST_F(SharedPtrTest, construct_WeakPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
    infra::WeakPtr<MySharedObject> weakObject(object);
    infra::WeakPtr<MySharedObject> weakObject2 = object;
    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, copy_construct_WeakPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
    infra::WeakPtr<MySharedObject> weakObject(object);
    infra::WeakPtr<MySharedObjectBase> weakObject2(weakObject);
    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, copy_assign_WeakPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
    infra::WeakPtr<MySharedObject> weakObject(object);
    infra::WeakPtr<MySharedObject> weakObject2;
    weakObject2 = weakObject;
    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, construct_SharedPtr_from_WeakPtr)
{
    infra::SharedPtr<MySharedObject> sharedObject;
    {
        EXPECT_CALL(objectConstructionMock, Construct(testing::_));
        infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
        infra::WeakPtr<MySharedObject> weakObject(object);
        sharedObject = weakObject.lock();
    }

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, construct_SharedPtr_from_empty_WeakPtr)
{
    infra::WeakPtr<MySharedObject> weakObject;
    infra::SharedPtr<MySharedObject> sharedObject = weakObject.lock();
    EXPECT_FALSE(sharedObject);
}

TEST_F(SharedPtrTest, convert_WeakPtr_to_SharedPtr)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
    infra::WeakPtr<MySharedObject> weakObject(object);
    infra::SharedPtr<MySharedObject> sharedObject(weakObject);
    infra::SharedPtr<MySharedObject> sharedObject2;
    sharedObject2 = weakObject;
    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, move_construct_WeakPtr)
{
    infra::SharedObjectAllocatorFixedSize<MySharedObject, void(infra::ConstructionMonitorMock&)>::WithStorage<1> allocator;

    infra::WeakPtr<MySharedObject> weakObject;

    {
        EXPECT_CALL(objectConstructionMock, Construct(testing::_));
        infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
        weakObject = object;
        EXPECT_CALL(objectConstructionMock, Destruct(testing::_));

        infra::WeakPtr<MySharedObject> weakObject2(std::move(weakObject));
    }
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    EXPECT_NE(nullptr, allocator.Allocate(objectConstructionMock));
}

TEST_F(SharedPtrTest, move_assign_WeakPtr)
{
    infra::SharedObjectAllocatorFixedSize<MySharedObject, void(infra::ConstructionMonitorMock&)>::WithStorage<1> allocator;

    infra::WeakPtr<MySharedObject> weakObject;

    {
        EXPECT_CALL(objectConstructionMock, Construct(testing::_));
        infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
        weakObject = object;
        EXPECT_CALL(objectConstructionMock, Destruct(testing::_));

        infra::WeakPtr<MySharedObject> weakObject2;
        weakObject2 = std::move(weakObject);
    }
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    EXPECT_NE(nullptr, allocator.Allocate(objectConstructionMock));
}

TEST_F(SharedPtrTest, converting_WeakPtr_to_SharedPtr_for_expired_object_results_in_nullptr)
{
    infra::SharedObjectAllocatorFixedSize<MySharedObject, void(infra::ConstructionMonitorMock&)>::WithStorage<1> allocator;

    infra::WeakPtr<MySharedObject> weakObject;

    {
        EXPECT_CALL(objectConstructionMock, Construct(testing::_));
        infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
        weakObject = object;
        EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    }

    infra::SharedPtr<MySharedObject> object(weakObject);
    EXPECT_EQ(nullptr, object);
}

TEST_F(SharedPtrTest, allocate_multiple_inherited_class)
{
    infra::SharedObjectAllocatorFixedSize<MultiInheritedClass, void(infra::ConstructionMonitorMock&)>::WithStorage<2> allocator;
    void* savedObject;
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).WillOnce(testing::SaveArg<0>(&savedObject));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
    EXPECT_TRUE(static_cast<bool>(object));
    EXPECT_CALL(objectConstructionMock, Destruct(savedObject));
}

TEST_F(SharedPtrTest, test_WeakPtr_equality)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).Times(2);
    infra::SharedPtr<MySharedObject> object1 = allocator.Allocate(objectConstructionMock);
    infra::SharedPtr<MySharedObject> object2 = allocator.Allocate(objectConstructionMock);
    infra::SharedPtr<MySharedObject> objectEmpty;
    infra::SharedPtr<MySharedObject> object1Same = object1;

    infra::WeakPtr<MySharedObject> weak1 = object1;
    infra::WeakPtr<MySharedObject> weak2 = object2;
    infra::WeakPtr<MySharedObject> weakEmpty = objectEmpty;
    infra::WeakPtr<MySharedObject> weak1Same = object1Same;

    EXPECT_TRUE(weak1 == weak1Same);
    EXPECT_FALSE(weak1 != weak1Same);
    EXPECT_FALSE(weak1 == weak2);
    EXPECT_TRUE(weak1 != weak2);
    EXPECT_FALSE(weak1 == weakEmpty);
    EXPECT_TRUE(weak1 != weakEmpty);

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_)).Times(2);
}

TEST_F(SharedPtrTest, test_WeakPtr_vs_SharedPtr_equality)
{
    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);
    infra::WeakPtr<MySharedObject> weak = object;

    EXPECT_TRUE(weak == object);
    EXPECT_FALSE(weak != object);
    EXPECT_TRUE(object == weak);
    EXPECT_FALSE(object != weak);

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
}

TEST_F(SharedPtrTest, destroy_self_reference)
{
    struct SelfReference
    {
        infra::SharedPtr<SelfReference> self;
    };

    infra::SharedObjectAllocatorFixedSize<SelfReference, void()>::WithStorage<1> allocator;
    infra::SharedPtr<SelfReference> sharedObject = allocator.Allocate();
    sharedObject->self = sharedObject;
    SelfReference& object = *sharedObject;
    sharedObject = nullptr;
    object.self = nullptr;
}

TEST_F(SharedPtrTest, construct_AccessedBySharedPtr)
{
    struct Object
    {
    } object;

    infra::MockCallback<void()> cb;
    infra::AccessedBySharedPtr sharedObject([&cb]() { cb.callback(); });

    {
        infra::WeakPtr<Object> weakObject(sharedObject.MakeShared(object));
        EXPECT_TRUE(sharedObject.Referenced());
        EXPECT_CALL(cb, callback());
    }

    EXPECT_FALSE(sharedObject.Referenced());
}

TEST_F(SharedPtrTest, construct_AccessedBySharedPtr_and_set_action_later)
{
    struct Object
    {
    } object;

    infra::MockCallback<void()> cb;
    infra::AccessedBySharedPtr sharedObject;

    sharedObject.SetAction([&cb]() { cb.callback(); });

    {
        infra::WeakPtr<Object> weakObject(sharedObject.MakeShared(object));
        EXPECT_CALL(cb, callback());
    }
}

TEST_F(SharedPtrTest, construct_non_owning_SharedPtr)
{
    struct Object
    {
        Object()
            : self(infra::UnOwnedSharedPtr(*this))
        {}

        infra::SharedPtr<Object> self;
    };

    infra::WeakPtr<Object> weakObject;

    {
        Object object;
        weakObject = object.self;

        infra::SharedPtr<Object> sharedObject(weakObject);
        EXPECT_TRUE(sharedObject != nullptr);
    }

    infra::SharedPtr<Object> sharedObject(weakObject);
    EXPECT_FALSE(sharedObject != nullptr);
}

TEST_F(SharedPtrTest, enable_shared_from_this)
{
    struct Object
        : public infra::EnableSharedFromThis<Object>
    {};

    infra::SharedObjectAllocatorFixedSize<Object, void()>::WithStorage<1> allocator;
    infra::SharedPtr<Object> object = allocator.Allocate();

    EXPECT_EQ(object, object->SharedFromThis());
    EXPECT_EQ(object, const_cast<const Object&>(*object).SharedFromThis());
    EXPECT_EQ(object, object->WeakFromThis());
    EXPECT_EQ(object, const_cast<const Object&>(*object).WeakFromThis());
}

TEST_F(SharedPtrTest, create_contained_SharedPtr)
{
    int x = 0;
    infra::SharedPtr<int> containedObject;

    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    containedObject = infra::MakeContainedSharedObject(x, object);

    object = nullptr;
    testing::Mock::VerifyAndClearExpectations(&objectConstructionMock);

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    containedObject = nullptr;
    testing::Mock::VerifyAndClearExpectations(&objectConstructionMock);
}

TEST_F(SharedPtrTest, create_moved_contained_SharedPtr)
{
    int x = 0;
    infra::SharedPtr<int> containedObject;

    EXPECT_CALL(objectConstructionMock, Construct(testing::_));
    infra::SharedPtr<MySharedObject> object = allocator.Allocate(objectConstructionMock);

    containedObject = infra::MakeContainedSharedObject(x, std::move(object));

    EXPECT_EQ(nullptr, object);
    testing::Mock::VerifyAndClearExpectations(&objectConstructionMock);

    EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    containedObject = nullptr;
    testing::Mock::VerifyAndClearExpectations(&objectConstructionMock);
}

TEST_F(SharedPtrTest, MakeSharedOnHeap)
{
    void* savedObject;
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).WillOnce(testing::SaveArg<0>(&savedObject));
    infra::SharedPtr<MySharedObject> object = infra::MakeSharedOnHeap<MySharedObject>(objectConstructionMock);
    EXPECT_TRUE(static_cast<bool>(object));
    EXPECT_EQ(5, object->Value());
    EXPECT_CALL(objectConstructionMock, Destruct(savedObject));
}
