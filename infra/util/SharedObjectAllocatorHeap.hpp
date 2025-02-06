#ifndef INFRA_SHARED_OBJECT_ALLOCATOR_HEAP_HPP
#define INFRA_SHARED_OBJECT_ALLOCATOR_HEAP_HPP

#include "infra/util/SharedObjectAllocator.hpp"

namespace infra
{
    template<class T, class ConstructionArgs>
    class SharedObjectAllocatorHeap;

    template<class T, class... ConstructionArgs>
    class SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>
        : public SharedObjectAllocator<T, void(ConstructionArgs...)>
        , private SharedObjectDeleter
    {
    public:
        ~SharedObjectAllocatorHeap();

        // Implementation of SharedObjectAllocator
        SharedPtr<T> Allocate(ConstructionArgs... args) override;
        void OnAllocatable(infra::AutoResetFunction<void()>&& callback) override;
        bool NoneAllocated() const override;

    private:
        // Implementation of SharedObjectDeleter
        void Destruct(const void* object) override;
        void Deallocate(void* control) override;

    private:
        struct Node
            : public detail::SharedPtrControl
        {
            explicit Node(SharedObjectDeleter* allocator);

            Node* next = nullptr;
            infra::StaticStorage<T> object{};
        };

    private:
        std::size_t numAllocated = 0;
        infra::AutoResetFunction<void()> onAllocatable;
    };

    ////    Implementation    ////

    template<class T, class... ConstructionArgs>
    SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::~SharedObjectAllocatorHeap()
    {
        assert(NoneAllocated());
    }

    template<class T, class... ConstructionArgs>
    SharedPtr<T> SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::Allocate(ConstructionArgs... args)
    {
        ++numAllocated;
        Node* node = new Node(this);
        node->object.Construct(std::forward<ConstructionArgs>(args)...);
        return SharedPtr<T>(node, &*node->object);
    }

    template<class T, class... ConstructionArgs>
    void SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::OnAllocatable(infra::AutoResetFunction<void()>&& callback)
    {
        onAllocatable = std::move(callback);
    }

    template<class T, class... ConstructionArgs>
    bool SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::NoneAllocated() const
    {
        return numAllocated == 0;
    }

    template<class T, class... ConstructionArgs>
    void SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::Destruct(const void* object)
    {
        reinterpret_cast<const StaticStorage<T>*>(object)->Destruct();
    }

    template<class T, class... ConstructionArgs>
    void SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::Deallocate(void* control)
    {
        Node* node = static_cast<Node*>(control);

        delete node;
        --numAllocated;

        if (onAllocatable != nullptr)
            onAllocatable();
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

    template<class T, class... ConstructionArgs>
    SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::Node::Node(SharedObjectDeleter* allocator)
        : detail::SharedPtrControl(&*object, allocator) //NOSONAR
    {}

#pragma GCC diagnostic pop
}

#endif
