#ifndef INFRA_UTIL_SINGLE_INSTANCE_HPP
#define INFRA_UTIL_SINGLE_INSTANCE_HPP

#include "infra/util/ReallyAssert.hpp"
#include <atomic>

namespace infra
{
    template<class Tag>
    class SingleInstance
    {
    public:
        SingleInstance()
        {
            really_assert_with_msg(!hasInstance, "Only single instance allowed");
            hasInstance = true;
        }

        SingleInstance(const SingleInstance&) = delete;
        SingleInstance(SingleInstance&&) = delete;
        SingleInstance& operator=(const SingleInstance&) = delete;
        SingleInstance& operator=(SingleInstance&&) = delete;

        ~SingleInstance()
        {
            hasInstance = false;
        }

#ifdef EMIL_HOST_BUILD
        static void ResetSingleInstanceCounter()
        {
            hasInstance = false;
        }
#endif

    private:
        static std::atomic<bool> hasInstance;
    };

    template<class Tag>
    std::atomic<bool> SingleInstance<Tag>::hasInstance = false;
}

#endif
