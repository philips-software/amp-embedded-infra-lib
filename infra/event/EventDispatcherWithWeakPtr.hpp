#ifndef INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_HPP
#define INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_HPP

#include "infra/event/EventDispatcher.hpp"
#include "infra/util/SharedPtr.hpp"

#ifndef INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_FUNCTION_EXTRA_SIZE
#define INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_FUNCTION_EXTRA_SIZE (INFRA_DEFAULT_FUNCTION_EXTRA_SIZE + (3 * sizeof(void*)))
#endif

namespace infra
{
    class EventDispatcherWithWeakPtrWorker
        : public EventDispatcherWorker
    {
    private:
        class Action
        {
        public:
            virtual ~Action() = default;

            virtual void Execute() = 0;
        };

        class ActionFunction
            : public Action
        {
        public:
            explicit ActionFunction(const Function<void()>& function);

            void Execute() override;

        private:
            Function<void()> function;
        };

        template<class T>
        class ActionWithWeakPtr
            : public Action
        {
        public:
            ActionWithWeakPtr(const infra::Function<void(const infra::SharedPtr<T>& object)>& function, const infra::WeakPtr<T>& object);

            void Execute() override;

        private:
            infra::Function<void(const infra::SharedPtr<T>& object)> function;
            infra::WeakPtr<T> object;
        };

    public:
        using ActionStorage = StaticStorageForPolymorphicObjects<Action, INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_FUNCTION_EXTRA_SIZE>;
        template<std::size_t StorageSize, class T = EventDispatcherWithWeakPtrWorker>
        using WithSize = infra::WithStorage<T, std::array<std::pair<ActionStorage, std::atomic<bool>>, StorageSize>>;

        explicit EventDispatcherWithWeakPtrWorker(MemoryRange<std::pair<ActionStorage, std::atomic<bool>>> scheduledActionsStorage);

        void Schedule(const infra::Function<void()>& action) override;

        template<class T>
        void Schedule(const typename std::decay<infra::Function<void(const infra::SharedPtr<T>& object)>>::type& action, const infra::SharedPtr<T>& object);
        template<class T>
        void Schedule(const typename std::decay<infra::Function<void(const infra::SharedPtr<T>& object)>>::type& action, const infra::WeakPtr<T>& object);

        void ExecuteFirstAction() override;
        std::size_t MinCapacity() const override;
        bool IsIdle() const override;

        void Run();
        void ExecuteAllActions();

    protected:
        virtual void RequestExecution();
        virtual void Idle();

    private:
        bool TryExecuteAction();

    private:
        infra::MemoryRange<std::pair<ActionStorage, std::atomic<bool>>> scheduledActions;
        std::atomic<uint32_t> scheduledActionsPushIndex{ 0 };
        uint32_t scheduledActionsPopIndex{ 0 };
        std::size_t minCapacity;
    };

    template<class T>
    class EventDispatcherWithWeakPtrConnector
        : public infra::InterfaceConnector<EventDispatcherWorker>
        , public infra::InterfaceConnector<EventDispatcherWithWeakPtrWorker>
        , public T
    {
    public:
        using infra::InterfaceConnector<EventDispatcherWithWeakPtrWorker>::Instance;

        template<std::size_t StorageSize>
        using WithSize = typename T::template WithSize<StorageSize, EventDispatcherWithWeakPtrConnector<T>>;

        template<class... ConstructionArgs>
        explicit EventDispatcherWithWeakPtrConnector(MemoryRange<std::pair<EventDispatcherWithWeakPtrWorker::ActionStorage, std::atomic<bool>>> scheduledActionsStorage, ConstructionArgs&&... args);
    };

    using EventDispatcherWithWeakPtr = EventDispatcherWithWeakPtrConnector<EventDispatcherWithWeakPtrWorker>;

    ////    Implementation    ////

    template<class T>
    void EventDispatcherWithWeakPtrWorker::Schedule(const typename std::decay<infra::Function<void(const infra::SharedPtr<T>& object)>>::type& action, const infra::SharedPtr<T>& object)
    {
        assert(object != nullptr);
        Schedule(action, infra::WeakPtr<T>(object));
    }

    template<class T>
    void EventDispatcherWithWeakPtrWorker::Schedule(const typename std::decay<infra::Function<void(const infra::SharedPtr<T>& object)>>::type& action, const infra::WeakPtr<T>& object)
    {
        uint32_t pushIndex = scheduledActionsPushIndex;
        uint32_t newPushIndex;

        do
        {
            newPushIndex = (pushIndex + 1) % scheduledActions.size();
        } while (!scheduledActionsPushIndex.compare_exchange_weak(pushIndex, newPushIndex));

        scheduledActions[pushIndex].first.Construct<ActionWithWeakPtr<T>>(action, object);
        assert(!scheduledActions[pushIndex].second);
        scheduledActions[pushIndex].second = true;

        minCapacity = std::min<std::size_t>(minCapacity, (scheduledActions.size() + scheduledActionsPopIndex - pushIndex - 1) % scheduledActions.size() + 1);
        assert(minCapacity >= 1);

        RequestExecution();
    }

    template<class T>
    EventDispatcherWithWeakPtrWorker::ActionWithWeakPtr<T>::ActionWithWeakPtr(const infra::Function<void(const infra::SharedPtr<T>& object)>& function, const infra::WeakPtr<T>& object)
        : function(function)
        , object(object)
    {}

    template<class T>
    void EventDispatcherWithWeakPtrWorker::ActionWithWeakPtr<T>::Execute()
    {
        infra::SharedPtr<T> sharedObject = object;
        if (sharedObject)
            function(sharedObject);
    }

    template<class T>
    template<class... ConstructionArgs>
    EventDispatcherWithWeakPtrConnector<T>::EventDispatcherWithWeakPtrConnector(MemoryRange<std::pair<EventDispatcherWithWeakPtrWorker::ActionStorage, std::atomic<bool>>> scheduledActionsStorage, ConstructionArgs&&... args)
        : infra::InterfaceConnector<EventDispatcherWorker>(this)
        , infra::InterfaceConnector<EventDispatcherWithWeakPtrWorker>(this)
        , T(scheduledActionsStorage, std::forward<ConstructionArgs>(args)...)
    {}
}

#endif
