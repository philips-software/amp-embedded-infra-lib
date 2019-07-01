#include "infra/util/SharedPtr.hpp"
#include "infra/util/SharedObjectAllocator.hpp"
#include <cassert>

namespace infra
{
    namespace detail
    {
        SharedPtrControl::SharedPtrControl(const void* object, SharedObjectDeleter* deleter)
            : object(object)
            , deleter(deleter)
        {}

        void SharedPtrControl::IncreaseSharedCount()
        {
            ++sharedPtrCount;
            IncreaseWeakCount();
        }

        void SharedPtrControl::DecreaseSharedCount()
        {
            assert(sharedPtrCount != 0);

            --sharedPtrCount;
            if (sharedPtrCount == 0)
                deleter->Destruct(object);

            DecreaseWeakCount();
        }

        void SharedPtrControl::IncreaseWeakCount()
        {
            ++weakPtrCount;
        }

        void SharedPtrControl::DecreaseWeakCount()
        {
            assert(weakPtrCount != 0);

            --weakPtrCount;
            if (weakPtrCount == 0)
                deleter->Deallocate(this);
        }

        bool SharedPtrControl::Expired() const
        {
            return sharedPtrCount == 0;
        }

        bool SharedPtrControl::UnReferenced() const
        {
            return weakPtrCount == 0;
        }

        void NullAllocator::Destruct(const void* object)
        {}

        void NullAllocator::Deallocate(void* control)
        {}
    }
}
