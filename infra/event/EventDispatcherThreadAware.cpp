#include "infra/event/EventDispatcherThreadAware.hpp"

namespace infra
{
    void EventDispatcherThreadAwareWorker::RequestExecution()
    {
        std::lock_guard lock{ mutex };

        ready = true;
        condition.notify_one();
    }

    void EventDispatcherThreadAwareWorker::Idle()
    {
        std::unique_lock lock{ mutex };

        condition.wait(lock, [this]()
            {
                return ready;
            });

        ready = false;
    }
}
