#ifndef INFRA_UTIL_SINGLE_INSTANCE_HPP
#define INFRA_UTIL_SINGLE_INSTANCE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    template<class Tag>
    class SingleInstance
    {
        static int numberOfInstances;

    public:
        SingleInstance()
        {
            numberOfInstances++;
            really_assert_with_msg(numberOfInstances <= 1, "Only single instance allowed");
        }

        SingleInstance(const SingleInstance&) = delete;
        SingleInstance(SingleInstance&&) = delete;
        SingleInstance& operator=(const SingleInstance&) = delete;
        SingleInstance& operator=(SingleInstance&&) = delete;

        ~SingleInstance()
        {
            numberOfInstances--;
            really_assert(numberOfInstances >= 0);
        }

#ifdef EMIL_HOST_BUILD
        static void ResetSingleInstanceCounter()
        {
            numberOfInstances = 0;
        }
#endif
    };

    template<class Tag>
    int SingleInstance<Tag>::numberOfInstances = 0;
}

#endif // INFRA_UTIL_SINGLE_INSTANCE_HPP
