#ifndef INFRA_UTIL_SINGLE_INSTANCE_HPP
#define INFRA_UTIL_SINGLE_INSTANCE_HPP

#include "infra/util/LogAndAbort.hpp"
#include <atomic>

namespace infra
{
    template<class Tag>
    class SingleInstance
    {
    public:
        SingleInstance()
        {
            if (hasInstance.exchange(true, std::memory_order_acquire))
                LOG_AND_ABORT("Only single instance allowed");
        }

        SingleInstance(const SingleInstance&) = delete;
        SingleInstance(SingleInstance&&) = delete;
        SingleInstance& operator=(const SingleInstance&) = delete;
        SingleInstance& operator=(SingleInstance&&) = delete;

    protected:
        ~SingleInstance()
        {
            hasInstance.store(false, std::memory_order_release);
        }

    private:
        static std::atomic<bool> hasInstance;
    };

    template<class Tag>
    std::atomic<bool> SingleInstance<Tag>::hasInstance = false;
}

#endif
