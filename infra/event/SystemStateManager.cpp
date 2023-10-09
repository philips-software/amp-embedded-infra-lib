#include "infra/event/SystemStateManager.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace infra
{
    SystemStateBase::SystemStateBase(void* aIdentifier)
        : identifier(aIdentifier)
    {}

    bool SystemStateBase::operator==(const SystemStateBase& other) const
    {
        return identifier == other.identifier;
    }

    bool SystemStateBase::operator!=(const SystemStateBase& other) const
    {
        return !(*this == other);
    }

    SystemStateParticipant::SystemStateParticipant(SystemStateManager& aSystemStateManager)
        : systemStateManager(aSystemStateManager)
    {
        systemStateManager.AddParticipant(*this);
    }

    SystemStateParticipant::~SystemStateParticipant()
    {
        systemStateManager.RemoveParticipant(*this);
    }

    void SystemStateParticipant::ReachedState()
    {
        infra::EventDispatcher::Instance().Schedule([this]()
            {
                systemStateManager.ReachedState(*this);
            });
    }

    void SystemStateManager::RunStates(infra::MemoryRange<const SystemStateBase> states)
    {
        assert(runningStates.empty() || currentState == std::prev(runningStates.end()));
        runningStates = states;
        currentState = std::prev(runningStates.begin());
        GoToNextState();
    }

    void SystemStateManager::AddParticipant(SystemStateParticipant& participant)
    {
        assert(runningStates.empty());
        participantsInCurrentState.push_back(participant);
    }

    void SystemStateManager::RemoveParticipant(SystemStateParticipant& participant)
    {
        assert(participantsInPreviousState.has_element(participant));
        participantsInPreviousState.erase(participant);
    }

    void SystemStateManager::ReachedState(SystemStateParticipant& participant)
    {
        assert(participantsInPreviousState.has_element(participant));
        participantsInPreviousState.erase(participant);
        participantsInCurrentState.push_back(participant);

        if (participantsInPreviousState.empty())
            GoToNextState();
    }

    void SystemStateManager::GoToNextState()
    {
        assert(participantsInPreviousState.empty());
        participantsInPreviousState = std::move(participantsInCurrentState);
        ++currentState;

        if (currentState != runningStates.end())
            InformAllParticipants();
    }

    void SystemStateManager::InformAllParticipants()
    {
        if (participantsInPreviousState.empty())
            GoToNextState();
        else
            for (SystemStateParticipant& participant : participantsInPreviousState)
                participant.RequestState(*currentState);
    }
}
