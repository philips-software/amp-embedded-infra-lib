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
    infra::ExecuteLogAndAbortHook(nullptr, nullptr, 0, "fool of a Took");
    infra::ExecuteLogAndAbortHook(nullptr, nullptr, 0, "speak %s and enter", "friend");
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
    infra::ExecuteLogAndAbortHook("reason", nullptr, 0, "speak %s and enter", "friend");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("reason"));
    EXPECT_THAT(output, testing::HasSubstr("speak friend and enter"));
}

TEST(LogAndAbortTest, log_and_abort_with_filename_prints_filename)
{
    infra::RegisterLogAndAbortHook([&](auto format, auto args)
        {
            PrintLogAndAbortToStdout(format, args);
        });

    testing::internal::CaptureStdout();
    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("reason", "filename.cpp", 32, "speak %s and enter", "friend");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("reason"));
    EXPECT_THAT(output, testing::HasSubstr("filename.cpp:32"));
    EXPECT_THAT(output, testing::HasSubstr("speak friend and enter"));
}

TEST(LogAndAbortTest, log_and_abort_recursive_call_skipped)
{
    infra::RegisterLogAndAbortHook([&](auto format, auto args)
        {
            PrintLogAndAbortToStdout(format, args);
            infra::ExecuteLogAndAbortHook("recursive", "file.cpp", 42, "recursive %s", "call");
        });

    testing::internal::CaptureStdout();
    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("condition", "file.cpp", 32, "initial %s", "call");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("condition"));
    EXPECT_THAT(output, testing::Not(testing::HasSubstr("recursive")));
}
