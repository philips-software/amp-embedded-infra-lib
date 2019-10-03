#ifndef SERVICES_STOPPABLE_HPP
#define SERVICES_STOPPABLE_HPP

#include "infra/util/Function.hpp"

namespace services
{
    class Stoppable
    {
    protected:
        Stoppable() = default;
        Stoppable(const Stoppable& other) = delete;
        Stoppable& operator=(const Stoppable& other) = delete;
        ~Stoppable() = default;

    public:
        virtual void Stop(const infra::Function<void()>& onDone) = 0;
    };
}

#endif
