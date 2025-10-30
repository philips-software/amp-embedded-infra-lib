#ifndef HAL_SLEEPABLE_HPP
#define HAL_SLEEPABLE_HPP

#include "infra/util/Function.hpp"

namespace hal
{
    class Sleepable
    {
    public:
        virtual void Sleep(const infra::Function<void()>& onDone) = 0;
        virtual void Wake(const infra::Function<void()>& onDone) = 0;
    };
}

#endif
