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
    {
    public:
        virtual SharedPtr<T> Allocate(ConstructionArgs... args) override;
    };

    ////    Implementation    ////

    template<class T, class... ConstructionArgs>
    SharedPtr<T> SharedObjectAllocatorHeap<T, void(ConstructionArgs...)>::Allocate(ConstructionArgs... args)
    {
        return MakeSharedOnHeap<T>(std::forward<ConstructionArgs>(args)...);
    }
}

#endif
