#include "gtest/gtest.h"
#include "infra/event/EventDispatcher.hpp"
#include "infra/event/SystemStateManager.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Optional.hpp"

class SystemStateTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    infra::SystemStateManager manager;
};

class TestParticipant
    : public infra::SystemStateParticipant
{
public:
    TestParticipant(infra::SystemStateManager& manager)
        : infra::SystemStateParticipant(manager)
    {}

    void RequestState(infra::SystemStateBase state)
    {
        requestedState = state;
    }

    using infra::SystemStateParticipant::ReachedState;

    infra::Optional<infra::SystemStateBase> requestedState;
};

TEST_F(SystemStateTest, Construction)
{}

TEST_F(SystemStateTest, RunSingleStateInEmptyManager)
{
    static const std::array<infra::SystemStateBase, 1> singleStateSequence = { infra::StateInitializing() };
    manager.RunStates(singleStateSequence);
}

TEST_F(SystemStateTest, RunSingleStateWithOneParticipant)
{
    TestParticipant participant(manager);

    static const std::array<infra::SystemStateBase, 1> singleStateSequence = { infra::StateInitializing() };
    manager.RunStates(singleStateSequence);

    EXPECT_EQ(infra::StateInitializing(), participant.requestedState);
    participant.ReachedState();
    ExecuteAllActions();
}

TEST_F(SystemStateTest, RunTwoStatesWithOneParticipant)
{
    TestParticipant participant(manager);

    static const std::array<infra::SystemStateBase, 2> twoStateSequence = { infra::StateInitializing(), infra::StateRunning() };
    manager.RunStates(twoStateSequence);

    participant.ReachedState();
    ExecuteAllActions();

    EXPECT_EQ(infra::StateRunning(), participant.requestedState);
    participant.ReachedState();
    ExecuteAllActions();
}

TEST_F(SystemStateTest, RunTwoStatesWithTwoParticipants)
{
    TestParticipant participant1(manager);
    TestParticipant participant2(manager);

    static const std::array<infra::SystemStateBase, 2> twoStateSequence = { infra::StateInitializing(), infra::StateRunning() };
    manager.RunStates(twoStateSequence);

    participant1.ReachedState();
    ExecuteAllActions();

    EXPECT_EQ(infra::StateInitializing(), participant1.requestedState);
    EXPECT_EQ(infra::StateInitializing(), participant2.requestedState);

    participant2.ReachedState();
    ExecuteAllActions();

    EXPECT_EQ(infra::StateRunning(), participant1.requestedState);
    EXPECT_EQ(infra::StateRunning(), participant2.requestedState);
    participant1.ReachedState();
    participant2.ReachedState();
    ExecuteAllActions();
}
