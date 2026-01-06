#include "infra/util/LogAndAbort.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <array>
#include <cstdio>

namespace
{
    void PrintLogAndAbortToStdout(const char* format, va_list* args)
    {
        std::array<char, 256> buffer;
        vsnprintf(buffer.data(), buffer.size(), format, *args);
        std::cout << buffer.data();
    }
}

TEST(LogAndAbortTest, log_and_abort_aborts)
{
    EXPECT_DEATH(LOG_AND_ABORT("Fly you fools!"), "");
}

TEST(LogAndAbortTest, log_and_abort_no_handler_does_nothing)
{
    testing::internal::CaptureStdout();
    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("fool of a took");
    infra::ExecuteLogAndAbortHook("speak %s and enter", "friend");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output.empty(), true);
}

TEST(LogAndAbortTest, log_and_abort_with_handler_calls_handler)
{
    infra::RegisterLogAndAbortHook([&](auto format, auto args)
        {
            PrintLogAndAbortToStdout(format, args);
        });

    testing::internal::CaptureStdout();
    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("speak %s and enter", "friend");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("speak friend and enter"));
}
