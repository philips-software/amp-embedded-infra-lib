#ifndef INFRA_REFERENCE_COUNTED_SINGLETON_HPP
#define INFRA_REFERENCE_COUNTED_SINGLETON_HPP

#include "infra/util/StaticStorage.hpp"
#include <cassert>

// This class helps creating a singleton that is constructed and destroyed based on a reference count.
// This reference count is controlled by creating objects of a nested Access type, via which the access
// to the singleton is controlled as well.

// Example for ReferenceCountedSingleton:
//
// Suppose we have a class InterruptDispatcher which is a singleton, and a class A which uses the singleton InterruptDispatcher
//
// class InterruptDispatcher
//     : public ReferenceCountedSingleton<InterruptDispatcher>
// { void DoSomething(); };
//
// class A
// {
//     InterruptDispatcher::Access interruptDispatcher;
//
//     void f()
//     {
//         interruptDispatcher->DoSomething();
//     }
// };

namespace infra
{

    template<class CRTP>
    class ReferenceCountedSingleton
    {
    public:
        class Access
        {
        public:
            Access();
            ~Access();

            Access(const Access&) = delete;
            Access& operator=(const Access&) = delete;

            CRTP& operator*();
            const CRTP& operator*() const;
            CRTP* operator->();
            const CRTP* operator->() const;
        };

        static CRTP& Instance();

    private:
        static uint32_t counter;

        struct StorageHelper
        {
            static infra::StaticStorage<CRTP> storage;
        };
    };

    template<class CRTP>
    uint32_t ReferenceCountedSingleton<CRTP>::counter = 0;

    template<class CRTP>
    infra::StaticStorage<CRTP> ReferenceCountedSingleton<CRTP>::StorageHelper::storage;

    //// Implementation ////

    template<class CRTP>
    CRTP& ReferenceCountedSingleton<CRTP>::Instance()
    {
        assert(counter != 0);
        return *StorageHelper::storage;
    }

    template<class CRTP>
    ReferenceCountedSingleton<CRTP>::Access::Access()
    {
        if (counter == 0)
            StorageHelper::storage.Construct();

        ++counter;
    }

    template<class CRTP>
    ReferenceCountedSingleton<CRTP>::Access::~Access()
    {
        --counter;

        if (counter == 0)
            StorageHelper::storage.Destruct();
    }

    template<class CRTP>
    CRTP& ReferenceCountedSingleton<CRTP>::Access::operator*()
    {
        return *StorageHelper::storage;
    }

    template<class CRTP>
    const CRTP& ReferenceCountedSingleton<CRTP>::Access::operator*() const
    {
        return *StorageHelper::storage;
    }

    template<class CRTP>
    CRTP* ReferenceCountedSingleton<CRTP>::Access::operator->()
    {
        return &*StorageHelper::storage;
    }

    template<class CRTP>
    const CRTP* ReferenceCountedSingleton<CRTP>::Access::operator->() const
    {
        return &*StorageHelper::storage;
    }

}

#endif
