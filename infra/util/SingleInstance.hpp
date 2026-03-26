#ifndef INFRA_UTIL_SINGLE_INSTANCE_HPP
#define INFRA_UTIL_SINGLE_INSTANCE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    template<class Tag>
    class SingleInstance
    {
        static int& NumberOfInstances()
        {
            static int numberOfInstances = 0;
            return numberOfInstances;
        }

    public:
        SingleInstance()
        {
            auto& numberOfInstances = NumberOfInstances();
            numberOfInstances++;
            really_assert_with_msg(numberOfInstances <= 1, "Only single instance allowed");
        }

        ~SingleInstance()
        {
            auto& numberOfInstances = NumberOfInstances();
            numberOfInstances--;
            really_assert(numberOfInstances >= 0);
        }

#ifdef EMIL_HOST_BUILD
        static void ResetSingleInstanceCounter()
        {
            NumberOfInstances() = 0;
        }
#endif
    };
}

#endif // INFRA_UTIL_SINGLE_INSTANCE_HPP
