#ifndef INFRA_SYSTEM_STATE_MANAGER_HPP
#define INFRA_SYSTEM_STATE_MANAGER_HPP

#include "infra/util/IntrusiveList.hpp"
#include "infra/util/MemoryRange.hpp"
#include <cstdint>

namespace infra
{
    class SystemStateBase
    {
    protected:
        explicit SystemStateBase(void* aIdentifier);

    public:
        bool operator==(const SystemStateBase& other) const;
        bool operator!=(const SystemStateBase& other) const;

    private:
        void* identifier;
    };

    template<class Derived>
    class SystemState
        : public SystemStateBase
    {
    public:
        SystemState();

    private:
        static uint8_t unique;
    };

    class SystemStateManager;

    class SystemStateParticipant
        : public IntrusiveList<SystemStateParticipant>::NodeType
    {
    public:
        explicit SystemStateParticipant(SystemStateManager& aSystemStateManager);
        ~SystemStateParticipant();

    protected:
        friend class SystemStateManager;

        virtual void RequestState(SystemStateBase state) = 0;
        void ReachedState();

    private:
        SystemStateManager& systemStateManager;
    };

    class SystemStateManager
    {
    public:
        SystemStateManager() = default;
        SystemStateManager(const SystemStateManager& other) = delete;
        SystemStateManager& operator=(const SystemStateManager& other) = delete;

        void RunStates(infra::MemoryRange<const SystemStateBase> states);

    private:
        friend class SystemStateParticipant;

        void AddParticipant(SystemStateParticipant& participant);
        void RemoveParticipant(SystemStateParticipant& participant);
        void ReachedState(SystemStateParticipant& participant);

    private:
        void GoToNextState();
        void InformAllParticipants();

    private:
        infra::IntrusiveList<SystemStateParticipant> participantsInPreviousState;
        infra::IntrusiveList<SystemStateParticipant> participantsInCurrentState;

        infra::MemoryRange<const SystemStateBase> runningStates;
        const SystemStateBase* currentState = nullptr;
    };

    struct StateInitializing: SystemState<StateInitializing> {};
    struct StateRunning: SystemState<StateRunning> {};

    static const std::array<SystemStateBase, 2> simpleStateSequence = { StateInitializing(), StateRunning() };

    ////    Implementation    ////

    template<class Derived>
    SystemState<Derived>::SystemState()
        : SystemStateBase(&unique)
    {}

    template<class Derived>
    uint8_t SystemState<Derived>::unique = 0;
}

#endif
