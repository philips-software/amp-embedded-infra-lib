#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include <cassert>

namespace infra
{
    EventDispatcherWithWeakPtrWorker::EventDispatcherWithWeakPtrWorker(MemoryRange<std::pair<ActionStorage, std::atomic<bool>>> scheduledActionsStorage)
        : scheduledActions(scheduledActionsStorage)
        , scheduledActionsPushIndex(0)
        , scheduledActionsPopIndex(0)
        , minCapacity(scheduledActions.size())
    {
        for (auto& action : scheduledActions)
            action.second = false;
    }

    void EventDispatcherWithWeakPtrWorker::Schedule(const infra::Function<void()>& action)
    {
        uint32_t pushIndex = scheduledActionsPushIndex;
        uint32_t newPushIndex;

        do
        {
            newPushIndex = (pushIndex + 1) % scheduledActions.size();
        } while (!scheduledActionsPushIndex.compare_exchange_weak(pushIndex, newPushIndex));

        scheduledActions[pushIndex].first.Construct<ActionFunction>(action);
        really_assert(!scheduledActions[pushIndex].second);
        scheduledActions[pushIndex].second = true;

        minCapacity = std::min<std::size_t>(minCapacity, (scheduledActions.size() + scheduledActionsPopIndex - pushIndex - 1) % scheduledActions.size() + 1);
        really_assert(minCapacity >= 1);

        RequestExecution();
    }

    void EventDispatcherWithWeakPtrWorker::Run()
    {
        while (true)                                                                                            //TICS !CPP4127
        {
            ExecuteAllActions();
            Idle();
        }
    }

    void EventDispatcherWithWeakPtrWorker::ExecuteAllActions()
    {
        while (TryExecuteAction())
        {}
    }

    bool EventDispatcherWithWeakPtrWorker::IsIdle() const
    {
        return !scheduledActions[scheduledActionsPopIndex].second;
    }

    std::size_t EventDispatcherWithWeakPtrWorker::MinCapacity() const
    {
        return minCapacity;
    }

    void EventDispatcherWithWeakPtrWorker::RequestExecution()
    {}

    void EventDispatcherWithWeakPtrWorker::Idle()
    {}

    void EventDispatcherWithWeakPtrWorker::ExecuteFirstAction()
    {
        scheduledActions[scheduledActionsPopIndex].first->Execute();
    }

    bool EventDispatcherWithWeakPtrWorker::TryExecuteAction()
    {
        if (scheduledActions[scheduledActionsPopIndex].second)
        {
            ExecuteFirstAction();
            scheduledActions[scheduledActionsPopIndex].first.Destruct();
            scheduledActions[scheduledActionsPopIndex].second = false;
            scheduledActionsPopIndex = (scheduledActionsPopIndex + 1) % scheduledActions.size();
            return true;
        }
        else
            return false;
    }

    EventDispatcherWithWeakPtrWorker::ActionFunction::ActionFunction(const Function<void()>& function)
        : function(function)
    {}

    void EventDispatcherWithWeakPtrWorker::ActionFunction::Execute()
    {
        function();
    }
}
