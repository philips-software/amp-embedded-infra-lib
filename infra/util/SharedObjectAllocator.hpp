#ifndef INFRA_SHARED_OBJECT_ALLOCATOR_HPP
#define INFRA_SHARED_OBJECT_ALLOCATOR_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/SharedPtr.hpp"

namespace infra
{
    template<class T, class ConstructionArgs>
    class SharedObjectAllocator;

    template<class T, class... ConstructionArgs>
    class SharedObjectAllocator<T, void(ConstructionArgs...)>
    {
    public:
        template<template<class, class...> class Allocator>
        using UsingAllocator = Allocator<T, void(ConstructionArgs...)>;

    protected:
        ~SharedObjectAllocator() = default;

    public:
        virtual SharedPtr<T> Allocate(ConstructionArgs... args) = 0;

        // If Allocate() fails, this callback will be invoked once when Allocate() can succeed the next time
        virtual void OnAllocatable(infra::AutoResetFunction<void()>&& callback) = 0;
    };
}

#endif
