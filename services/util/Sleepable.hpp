#ifndef SERVICES_SLEEPABLE_HPP
#define SERVICES_SLEEPABLE_HPP

#include "infra/util/Function.hpp"

namespace services
{
    class Sleepable
    {
    public:
        virtual void Sleep(const infra::Function<void()>& onDone) = 0;
        virtual void Wake(const infra::Function<void()>& onDone) = 0;
    };
}

#endif
