#ifndef INFRA_EVENT_DISPATCHER_HPP
#define INFRA_EVENT_DISPATCHER_HPP

#include "infra/util/Function.hpp"
#include "infra/util/InterfaceConnector.hpp"
#include "infra/util/WithStorage.hpp"
#include <atomic>

namespace infra
{
    class EventDispatcherWorker
    {
    protected:
        EventDispatcherWorker() = default;
        EventDispatcherWorker(const EventDispatcherWorker& other) = delete;
        EventDispatcherWorker& operator=(const EventDispatcherWorker& other) = delete;
        ~EventDispatcherWorker() = default;

    public:
        virtual void Schedule(const infra::Function<void()>& action) = 0;
        virtual void ExecuteFirstAction() = 0;
        virtual std::size_t MinCapacity() const = 0;
        virtual bool IsIdle() const = 0;
    };

    class EventDispatcherWorkerImpl
        : public EventDispatcherWorker
    {
    public:
        template<std::size_t StorageSize, class T = EventDispatcherWorkerImpl>
            using WithSize = infra::WithStorage<T, std::array<std::pair<infra::Function<void()>, std::atomic<bool>>, StorageSize>>;

        explicit EventDispatcherWorkerImpl(MemoryRange<std::pair<infra::Function<void()>, std::atomic<bool>>> scheduledActionsStorage);

        virtual void Schedule(const infra::Function<void()>& action) override;
        virtual std::size_t MinCapacity() const override;
        virtual bool IsIdle() const override;

        void Run();
        void ExecuteAllActions();

    protected:
        virtual void RequestExecution();
        virtual void Idle();

        virtual void ExecuteFirstAction() override;

    private:
        bool TryExecuteAction();

    private:
        infra::MemoryRange<std::pair<infra::Function<void()>, std::atomic<bool>>> scheduledActions;
        std::atomic<uint32_t> scheduledActionsPushIndex;
        uint32_t scheduledActionsPopIndex;
        std::size_t minCapacity;
    };

    template<class T>
    class EventDispatcherConnector
        : public infra::InterfaceConnector<EventDispatcherWorker>
        , public T
    {
    public:
        template<std::size_t StorageSize>
            using WithSize = typename T::template WithSize<StorageSize, EventDispatcherConnector<T>>;

        template<class... ConstructionArgs>
            explicit EventDispatcherConnector(MemoryRange<std::pair<infra::Function<void()>, std::atomic<bool>>> scheduledActionsStorage, ConstructionArgs&&... args);
    };

    using EventDispatcher = EventDispatcherConnector<EventDispatcherWorkerImpl>;

    ////    Implementation    ////

    template<class T>
    template<class... ConstructionArgs>
    EventDispatcherConnector<T>::EventDispatcherConnector(MemoryRange<std::pair<infra::Function<void()>, std::atomic<bool>>> scheduledActionsStorage, ConstructionArgs&&... args)
        : infra::InterfaceConnector<EventDispatcherWorker>(this)
        , T(scheduledActionsStorage, std::forward(args)...)
    {}
}

#endif
