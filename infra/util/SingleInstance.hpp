#ifndef INFRA_UTIL_SINGLE_INSTANCE_HPP
#define INFRA_UTIL_SINGLE_INSTANCE_HPP

#include "infra/util/ReallyAssert.hpp"

namespace infra
{
    template<class Tag>
    class SingleInstance
    {
        int& GetNumberOfInstances()
        {
            static int numberOfInstances = 0;
            return numberOfInstances;
        }

    public:
        SingleInstance()
        {
            auto& numberOfInstances = GetNumberOfInstances();
            numberOfInstances++;
            really_assert_with_msg(numberOfInstances <= 1, "Only single instance allowed");
        }

        ~SingleInstance()
        {
            auto& numberOfInstances = GetNumberOfInstances();
            numberOfInstances--;
            really_assert(numberOfInstances >= 0);
        }
    };
}

#endif // INFRA_UTIL_SINGLE_INSTANCE_HPP
