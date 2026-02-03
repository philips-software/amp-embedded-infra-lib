#include "infra/util/LogAndAbort.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <array>
#include <cstdarg>
#include <cstdio>

namespace
{
    void PrintLogAndAbortToStdout(const char* reason, const char* file, int line, const char* format, va_list* args)
    {
        std::array<char, 256> buffer;
        vsnprintf(buffer.data(), buffer.size(), format, *args);
        std::cout << buffer.data();
    }

    class LogAndAbortTest
        : public testing::Test
    {
    public:
        ~LogAndAbortTest()
        {
            infra::RegisterLogAndAbortHook(nullptr);
        }

        testing::MockFunction<void(const char*, const char*, int, const char*, va_list*)> logAndAbortMock;
    };
}

TEST_F(LogAndAbortTest, log_and_abort_aborts)
{
    EXPECT_DEATH(LOG_AND_ABORT("Fly you fools!"), "");
}

TEST_F(LogAndAbortTest, log_and_abort_no_handler_does_nothing)
{
    testing::internal::CaptureStdout();
    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook(nullptr, nullptr, 0, "fool of a Took");
    infra::ExecuteLogAndAbortHook(nullptr, nullptr, 0, "speak %s and enter", "friend");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output.empty(), true);
}

TEST_F(LogAndAbortTest, log_and_abort_with_handler_calls_handler)
{
    infra::RegisterLogAndAbortHook([this](auto&&... args)
        {
            logAndAbortMock.Call(std::forward<decltype(args)>(args)...);
        });

    EXPECT_CALL(logAndAbortMock, Call("reason", nullptr, 0, "speak %s and enter", testing::_));

    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("reason", nullptr, 0, "speak %s and enter", "friend");
}

TEST_F(LogAndAbortTest, log_and_abort_with_filename_prints_filename)
{
    infra::RegisterLogAndAbortHook([this](auto&&... args)
        {
            logAndAbortMock.Call(std::forward<decltype(args)>(args)...);
        });

    EXPECT_CALL(logAndAbortMock, Call("reason", "filename.cpp", 32, "speak %s and enter", testing::_));

    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("reason", "filename.cpp", 32, "speak %s and enter", "friend");
}

TEST_F(LogAndAbortTest, log_and_abort_recursive_call_skipped)
{
    infra::RegisterLogAndAbortHook([this](auto&&... args)
        {
            logAndAbortMock.Call(std::forward<decltype(args)>(args)...);
            infra::ExecuteLogAndAbortHook("recursive", "file.cpp", 42, "recursive %s", "call");
        });

    EXPECT_CALL(logAndAbortMock, Call("condition", "file.cpp", 32, "initial %s", testing::_));

    // Manually calling hook to avoid aborting the test
    infra::ExecuteLogAndAbortHook("condition", "file.cpp", 32, "initial %s", "call");
}
