#include "infra/util/LogAndAbort.hpp"
#include "services/ble/StateGuard.hpp"
#include "gmock/gmock.h"
#include <cstdarg>
#include <cstdio>

namespace services
{
    namespace
    {
        class StateGuardMock
            : public StateGuard
        {
        public:
            MOCK_METHOD(GapState, DetermineCurrentState, (), (const, override));
        };

        class TestStateGuard
            : public testing::Test
        {
        public:
            ~TestStateGuard()
            {
                infra::RegisterLogAndAbortHook(nullptr);
            }

            void RegisterStderrHook()
            {
                infra::RegisterLogAndAbortHook([]([[maybe_unused]] const char* reason, [[maybe_unused]] const char* file, [[maybe_unused]] int line, const char* format, va_list* args)
                    {
                        std::vfprintf(stderr, format, *args);
                    });
            }

            testing::NiceMock<StateGuardMock> stateGuard;
        };
    }

    TEST_F(TestStateGuard, state_is_returns_true_when_current_state_matches)
    {
        ON_CALL(stateGuard, DetermineCurrentState()).WillByDefault(testing::Return(GapState::standby));

        EXPECT_THAT(stateGuard.StateIs({ GapState::standby }), testing::IsTrue());
    }

    TEST_F(TestStateGuard, state_is_returns_true_when_current_state_matches_one_of_many)
    {
        ON_CALL(stateGuard, DetermineCurrentState()).WillByDefault(testing::Return(GapState::connected));

        EXPECT_THAT(stateGuard.StateIs({ GapState::standby, GapState::connected }), testing::IsTrue());
    }

    TEST_F(TestStateGuard, state_is_returns_false_when_current_state_does_not_match)
    {
        ON_CALL(stateGuard, DetermineCurrentState()).WillByDefault(testing::Return(GapState::advertising));

        EXPECT_THAT(stateGuard.StateIs({ GapState::standby, GapState::connected }), testing::IsFalse());
    }

    TEST_F(TestStateGuard, assert_state_is_does_not_abort_when_state_matches)
    {
        ON_CALL(stateGuard, DetermineCurrentState()).WillByDefault(testing::Return(GapState::standby));

        stateGuard.AssertStateIs({ GapState::standby, GapState::connected });
#ifndef EMIL_MUTATION_TESTING
    }

    TEST_F(TestStateGuard, assert_state_is_aborts_with_message_for_single_expected_state)
    {
        ON_CALL(stateGuard, DetermineCurrentState()).WillByDefault(testing::Return(GapState::advertising));
        RegisterStderrHook();

        EXPECT_DEATH(stateGuard.AssertStateIs({ GapState::standby }), "Unexpected state found: 3, expected: \\[0\\]");
    }

    TEST_F(TestStateGuard, assert_state_is_aborts_with_message_for_multiple_expected_states)
    {
        ON_CALL(stateGuard, DetermineCurrentState()).WillByDefault(testing::Return(GapState::initiating));
        RegisterStderrHook();

        EXPECT_DEATH(stateGuard.AssertStateIs({ GapState::standby, GapState::connected }), "Unexpected state found: 4, expected: \\[0,2\\]");
    }
#endif
}
