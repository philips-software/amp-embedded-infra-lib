#ifndef INFRA_SHARED_OBJECT_ALLOCATOR_FIXED_SIZE_HPP
#define INFRA_SHARED_OBJECT_ALLOCATOR_FIXED_SIZE_HPP

#include "infra/util/BoundedVector.hpp"
#include "infra/util/SharedObjectAllocator.hpp"
#include "infra/util/StaticStorage.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    template<class T, class ConstructionArgs>
    class SharedObjectAllocatorFixedSize;

    template<class T, class... ConstructionArgs>
    class SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>
        : public SharedObjectAllocator<T, void(ConstructionArgs...)>
        , private SharedObjectDeleter
    {
        static_assert(sizeof(T) == sizeof(StaticStorage<T>), "sizeof(StaticStorage) must be equal to sizeof(T) else reinterpret_cast will fail");

    private:
        struct Node
            : public detail::SharedPtrControl
        {
            explicit Node(SharedObjectDeleter* allocator);

            Node* next = nullptr;
            infra::StaticStorage<T> object{};
        };

    public:
        template<std::size_t NumberOfElements>
        using WithStorage = infra::WithStorage<SharedObjectAllocatorFixedSize, typename infra::BoundedVector<Node>::template WithMaxSize<NumberOfElements>>;

        explicit SharedObjectAllocatorFixedSize(infra::BoundedVector<Node>& elements);
        SharedObjectAllocatorFixedSize(const SharedObjectAllocatorFixedSize& other) = delete;
        SharedObjectAllocatorFixedSize& operator=(const SharedObjectAllocatorFixedSize& other) = delete;
        ~SharedObjectAllocatorFixedSize();

        virtual SharedPtr<T> Allocate(ConstructionArgs... args) override;
        virtual void OnAllocatable(infra::AutoResetFunction<void()>&& callback) override;

    private:
        virtual void Destruct(const void* object) override;
        virtual void Deallocate(void* control) override;

    private:
        std::size_t FreeListSize() const;
        Node* AllocateNode();

    private:
        infra::BoundedVector<Node>& elements;
        Node* freeList = nullptr;
        infra::AutoResetFunction<void()> onAllocatable;
    };

    ////    Implementation    ////

    template<class T, class... ConstructionArgs>
    SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::SharedObjectAllocatorFixedSize(infra::BoundedVector<Node>& elements)
        : elements(elements)
    {}

    template<class T, class... ConstructionArgs>
    SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::~SharedObjectAllocatorFixedSize()
    {
        assert(FreeListSize() == elements.size());
    }

    template<class T, class... ConstructionArgs>
    SharedPtr<T> SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::Allocate(ConstructionArgs... args)
    {
        Node* node = AllocateNode();
        if (node)
        {
            node->object.Construct(std::forward<ConstructionArgs>(args)...);
            return SharedPtr<T>(node, &*node->object);
        }
        else
            return nullptr;
    }

    template<class T, class... ConstructionArgs>
    void SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::OnAllocatable(infra::AutoResetFunction<void()>&& callback)
    {
        onAllocatable = std::move(callback);
    }

    template<class T, class... ConstructionArgs>
    void SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::Destruct(const void* object)
    {
        reinterpret_cast<const StaticStorage<T>*>(object)->Destruct();
    }

    template<class T, class... ConstructionArgs>
    void SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::Deallocate(void* control)
    {
        Node* node = static_cast<Node*>(control);

        node->next = freeList;
        freeList = node;

        if (onAllocatable != nullptr)
            onAllocatable();
    }

    template<class T, class... ConstructionArgs>
    std::size_t SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::FreeListSize() const
    {
        Node* node = freeList;
        std::size_t result = 0;

        while (node)
        {
            node = node->next;
            ++result;
        }

        return result;
    }

    template<class T, class... ConstructionArgs>
    typename SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::Node* SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::AllocateNode()
    {
        if (freeList)
        {
            Node* result = freeList;
            freeList = freeList->next;
            return result;
        }
        else
        {
            if (elements.full())
                return nullptr;

            elements.emplace_back(static_cast<SharedObjectDeleter*>(this));
            return &elements.back();
        }
    }

    template<class T, class... ConstructionArgs>
    SharedObjectAllocatorFixedSize<T, void(ConstructionArgs...)>::Node::Node(SharedObjectDeleter* allocator)
        : detail::SharedPtrControl(&*object, allocator)
    {}
}

#endif
