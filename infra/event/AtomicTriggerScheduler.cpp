#include "infra/event/AtomicTriggerScheduler.hpp"
#include <cassert>

namespace infra
{
    void AtomicTriggerScheduler::Schedule(const infra::Function<void()>& action)
    {
        if (!scheduled.exchange(true))
        {
            scheduledAction = action;

            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    scheduled = false;
                    scheduledAction();
                });
        }
    }
}
