#ifndef INFRA_ATOMIC_TRIGGER_SCHEDULER_HPP
#define INFRA_ATOMIC_TRIGGER_SCHEDULER_HPP

#include "infra/event/EventDispatcher.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include <atomic>

namespace infra
{
    class AtomicTriggerScheduler
    {
    public:
        void Schedule(const infra::Function<void()>& action);

    private:
        std::atomic_bool scheduled{ false };
        infra::AutoResetFunction<void()> action;
    };
}

#endif
