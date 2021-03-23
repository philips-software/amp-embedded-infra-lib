#ifndef INFRA_ALLOCATOR_HEAP_HPP
#define INFRA_ALLOCATOR_HEAP_HPP

#include "infra/util/Allocator.hpp"

namespace infra
{
    template<class T, class ConstructionArgs>
    class AllocatorHeap;

    template<class T, class... ConstructionArgs>
    class AllocatorHeap<T, void(ConstructionArgs...)>
        : public Allocator<T, void(ConstructionArgs...)>
    {
    public:
        virtual UniquePtr<T> Allocate(ConstructionArgs... args) override;
        virtual void Deallocate(void* object) override;
    };

    ////    Implementation    ////

    template<class T, class... ConstructionArgs>
    UniquePtr<T> AllocatorHeap<T, void(ConstructionArgs...)>::Allocate(ConstructionArgs... args)
    {
        T* object = new (std::nothrow) T(args...);
        if (object != nullptr)
            return MakeUnique(object, *this);
        else
            return nullptr;
    }

    template<class T, class... ConstructionArgs>
    void AllocatorHeap<T, void(ConstructionArgs...)>::Deallocate(void* object)
    {
        delete static_cast<T*>(object);
    }
}

#endif
