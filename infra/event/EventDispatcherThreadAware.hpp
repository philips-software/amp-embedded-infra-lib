#ifndef INFRA_EVENT_DISPATCHER_THREAD_AWARE_HPP
#define INFRA_EVENT_DISPATCHER_THREAD_AWARE_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include <condition_variable>
#include <mutex>

namespace infra
{
    class EventDispatcherThreadAwareWorker
        : public infra::EventDispatcherWithWeakPtrWorker
    {
    public:
        using EventDispatcherWithWeakPtrWorker::EventDispatcherWithWeakPtrWorker;

        void RequestExecution() override;
        void Idle() override;

    private:
        std::condition_variable condition;
        std::mutex mutex;
        bool ready{ false };
    };

    using EventDispatcherThreadAware = EventDispatcherWithWeakPtrConnector<EventDispatcherThreadAwareWorker>;
}

#endif
