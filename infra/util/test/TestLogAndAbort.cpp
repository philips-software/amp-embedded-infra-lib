#include "infra/util/LogAndAbort.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace
{
    class Listener
    {
    public:
        MOCK_METHOD(void, LogAndAbortHook, (const char* message, va_list args), ());
    };

    class LogAndAbortTest
        : public testing::Test
    {
    public:
        Listener listener;
    };
}

TEST_F(LogAndAbortTest, log_and_abort_aborts)
{
    EXPECT_DEATH(LOG_AND_ABORT("Fly you fools!"), "");
}

TEST_F(LogAndAbortTest, log_and_abort_no_handler_does_nothing)
{
    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("fool of a took");
    infra::ExecuteLogAndAbortHook("speak %s and enter", "friend");
}

TEST_F(LogAndAbortTest, log_and_abort_with_handler_calls_handler)
{
    infra::RegisterLogAndAbortHook([&](auto format, auto args)
        {
            listener.LogAndAbortHook(format, *args);
        });
    EXPECT_CALL(listener, LogAndAbortHook(testing::_, testing::_)).Times(1);

    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("speak %s and enter", "friend");
}
