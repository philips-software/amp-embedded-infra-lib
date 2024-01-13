#include "infra/event/EventDispatcher.hpp"
#include <cassert>

namespace infra
{
    EventDispatcherWorkerImpl::EventDispatcherWorkerImpl(MemoryRange<std::pair<infra::Function<void()>, std::atomic<bool>>> scheduledActionsStorage)
        : scheduledActions(scheduledActionsStorage)
        , minCapacity(scheduledActions.size())
    {
        for (auto& action : scheduledActions)
            action.second = false;
    }

    void EventDispatcherWorkerImpl::Schedule(const infra::Function<void()>& action)
    {
        uint32_t pushIndex = scheduledActionsPushIndex;
        uint32_t newPushIndex;

        do
        {
            newPushIndex = (pushIndex + 1) % scheduledActions.size();
        } while (!scheduledActionsPushIndex.compare_exchange_weak(pushIndex, newPushIndex));

        scheduledActions[pushIndex].first = action;
        really_assert(!scheduledActions[pushIndex].second);
        scheduledActions[pushIndex].second = true;

        minCapacity = std::min<std::size_t>(minCapacity, (scheduledActions.size() + scheduledActionsPopIndex - pushIndex - 1) % scheduledActions.size() + 1);
        really_assert(minCapacity >= 1);

        RequestExecution();
    }

    void EventDispatcherWorkerImpl::Run()
    {
        while (true)
        {
            ExecuteAllActions();
            Idle();
        }
    }

    void EventDispatcherWorkerImpl::ExecuteAllActions()
    {
        while (TryExecuteAction())
        {}
    }

    void EventDispatcherWorkerImpl::ExecuteUntil(const infra::Function<bool()>& predicate)
    {
        while (!predicate())
        {
            ExecuteAllActions();

            if (predicate())
                break;

            Idle();
        }
    }

    void EventDispatcherWorkerImpl::ExecuteFirstAction()
    {
        if (scheduledActions[scheduledActionsPopIndex].second)
        {
            scheduledActions[scheduledActionsPopIndex].first();
            scheduledActions[scheduledActionsPopIndex].first = nullptr;
            scheduledActions[scheduledActionsPopIndex].second = false;
            scheduledActionsPopIndex = (scheduledActionsPopIndex + 1) % scheduledActions.size();
        }
    }

    bool EventDispatcherWorkerImpl::IsIdle() const
    {
        return !scheduledActions[scheduledActionsPopIndex].second;
    }

    std::size_t EventDispatcherWorkerImpl::MinCapacity() const
    {
        return minCapacity;
    }

    void EventDispatcherWorkerImpl::RequestExecution()
    {}

    void EventDispatcherWorkerImpl::Idle()
    {}

    bool EventDispatcherWorkerImpl::TryExecuteAction()
    {
        if (scheduledActions[scheduledActionsPopIndex].second)
        {
            ExecuteFirstAction();
            return true;
        }
        else
            return false;
    }
}
