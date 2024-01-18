#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include <cassert>

namespace infra
{
    EventDispatcherWithWeakPtrWorker::EventDispatcherWithWeakPtrWorker(MemoryRange<std::pair<ActionStorage, std::atomic<bool>>> scheduledActionsStorage)
        : scheduledActions(scheduledActionsStorage)
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
        while (true)
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

    void EventDispatcherWithWeakPtrWorker::ExecuteUntil(const infra::Function<bool()>& predicate)
    {
        while (!predicate())
        {
            ExecuteAllActions();

            if (predicate())
                break;

            Idle();
        }
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
        if (scheduledActions[scheduledActionsPopIndex].second)
        {
            struct ExceptionSafePop
            {
                ExceptionSafePop(const ExceptionSafePop&) = delete;
                ExceptionSafePop& operator=(const ExceptionSafePop&) = delete;

                ~ExceptionSafePop()
                {
                    worker.scheduledActions[worker.scheduledActionsPopIndex].first.Destruct();
                    worker.scheduledActions[worker.scheduledActionsPopIndex].second = false;
                    worker.scheduledActionsPopIndex = (worker.scheduledActionsPopIndex + 1) % worker.scheduledActions.size();
                }

                EventDispatcherWithWeakPtrWorker& worker;
            };

            ExceptionSafePop popAction{ *this };
            scheduledActions[scheduledActionsPopIndex].first->Execute();
        }
    }

    bool EventDispatcherWithWeakPtrWorker::TryExecuteAction()
    {
        if (scheduledActions[scheduledActionsPopIndex].second)
        {
            ExecuteFirstAction();
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
