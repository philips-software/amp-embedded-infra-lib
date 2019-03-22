#include "infra/event/LowPowerEventDispatcher.hpp"

namespace infra
{
    MainClockReference::MainClockReference()
        : numReferenced(0)
    {}

    void MainClockReference::Refere()
    {
        ++numReferenced;
    }

    void MainClockReference::Release()
    {
        --numReferenced;
    }

    bool MainClockReference::IsReferenced() const
    {
        return numReferenced != 0;
    }

    LowPowerEventDispatcherWorker::LowPowerEventDispatcherWorker(infra::MemoryRange<std::pair<infra::EventDispatcherWithWeakPtrWorker::ActionStorage, std::atomic<bool>>> scheduledActionsStorage, LowPowerStrategy& lowPowerStrategy)
        : infra::EventDispatcherWithWeakPtrWorker(scheduledActionsStorage)
        , lowPowerStrategy(lowPowerStrategy)
    {}

    void LowPowerEventDispatcherWorker::RequestExecution()
    {
        lowPowerStrategy.RequestExecution();
    }

    void LowPowerEventDispatcherWorker::Idle()
    {
        lowPowerStrategy.Idle(*this);
    }
}
