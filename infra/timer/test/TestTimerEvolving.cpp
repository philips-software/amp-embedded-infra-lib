#include "gtest/gtest.h"
#include "infra/timer/TimerEvolving.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"

class TimerEvolvingTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    MOCK_METHOD1(Evolve, infra::Optional<infra::Duration>(uint32_t));
    MOCK_METHOD0(Action, void());
};

TEST_F(TimerEvolvingTest, explicit_start)
{
    infra::TimerEvolving timer;

    EXPECT_CALL(*this, Evolve(0)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    timer.Start([this](uint32_t index) { return Evolve(index); }, [this]() { Action(); });

    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(1)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(TimerEvolvingTest, start_by_constructor)
{
    EXPECT_CALL(*this, Evolve(0)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    infra::TimerEvolving timer([this](uint32_t index) { return Evolve(index); }, [this]() { Action(); });

    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(1)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(TimerEvolvingTest, timer_is_cancelled_after_returning_none)
{
    EXPECT_CALL(*this, Evolve(0)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    infra::TimerEvolving timer([this](uint32_t index) { return Evolve(index); }, [this]() { Action(); });

    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(1)).WillOnce(testing::Return(infra::none));
    ForwardTime(std::chrono::seconds(100));
}

TEST_F(TimerEvolvingTest, timer_follows_evolution)
{
    EXPECT_CALL(*this, Evolve(0)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    infra::TimerEvolving timer([this](uint32_t index) { return Evolve(index); }, [this]() { Action(); });

    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(1)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(5))));
    ForwardTime(std::chrono::seconds(1));

    ForwardTime(std::chrono::seconds(4));
    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(2)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(10))));
    ForwardTime(std::chrono::seconds(1));

    ForwardTime(std::chrono::seconds(9));
    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(3)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    ForwardTime(std::chrono::seconds(1));

    EXPECT_CALL(*this, Action());
    EXPECT_CALL(*this, Evolve(4)).WillOnce(testing::Return(infra::MakeOptional<infra::Duration>(std::chrono::seconds(1))));
    ForwardTime(std::chrono::seconds(1));
}
