#ifndef LOW_POWER_EVENT_DISPATCHER
#define LOW_POWER_EVENT_DISPATCHER

#include "infra/event/EventDispatcherWithWeakPtr.hpp"

#include <atomic>

namespace infra
{
    class MainClockReference
    {
    public:
        MainClockReference();

        void Refere();
        void Release();
        bool IsReferenced() const;

    private:
        std::atomic<uint32_t> numReferenced;
    };

    class LowPowerStrategy
    {
    protected:
        LowPowerStrategy() = default;
        virtual ~LowPowerStrategy() = default;
        LowPowerStrategy(const LowPowerStrategy& other) = delete;
        LowPowerStrategy& operator=(const LowPowerStrategy& other) = delete;

    public:
        virtual void RequestExecution() = 0;
        virtual void Idle(const EventDispatcherWorker& eventDispatcher) = 0;
    };

    class LowPowerEventDispatcherWorker
        : public infra::EventDispatcherWithWeakPtrWorker
    {
    public:
        template<std::size_t StorageSize, class T = LowPowerEventDispatcherWorker>
            using WithSize = infra::EventDispatcherWithWeakPtrWorker::WithSize<StorageSize, T>;

        LowPowerEventDispatcherWorker(infra::MemoryRange<std::pair<infra::EventDispatcherWithWeakPtrWorker::ActionStorage, std::atomic<bool>>> scheduledActionsStorage, LowPowerStrategy& lowPowerStrategy);

    protected:
        virtual void RequestExecution() override;
        virtual void Idle() override;

    private:
        LowPowerStrategy& lowPowerStrategy;
    };

    using LowPowerEventDispatcher = infra::EventDispatcherWithWeakPtrConnector<LowPowerEventDispatcherWorker>;
}

#endif
