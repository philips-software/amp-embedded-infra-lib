#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/util/LogAndAbort.hpp"
#include "services/tracer/LogAndAbortTracer.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/test_doubles/FlushableMock.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace
{
    class LogAndAbortTracerTest
        : public testing::Test
    {
    public:
        services::FlushableMock flushable;
        infra::StdStringOutputStream::WithStorage stream;
        services::TracerToStream tracer{ stream };
    };
}

TEST_F(LogAndAbortTracerTest, log_and_abort_without_registered_tracer_doesnt_call_tracer)
{
    // Calling hook directly to avoid aborting the test process.
    infra::ExecuteLogAndAbortHook("reason", nullptr, 0, "speak %s and enter", "elf");

    EXPECT_THAT(stream.Storage().empty(), true);
}

TEST_F(LogAndAbortTracerTest, log_and_abort_with_tracer_calls_tracer)
{
    services::LogAndAbortTracer logAndAbortTracer(tracer);

    // Calling hook directly to avoid aborting the test process.
    infra::ExecuteLogAndAbortHook("reason", "filename.cpp", 32, "speak %s and enter", "friend");

    EXPECT_THAT(stream.Storage(), testing::StrEq("\r\n\r\nreason! [speak friend and enter] at filename.cpp:32\r\n"));
}

TEST_F(LogAndAbortTracerTest, log_and_abort_with_tracer_and_flushable_calls_tracer_and_flushes)
{
    services::LogAndAbortTracer logAndAbortTracer(tracer, flushable);

    EXPECT_CALL(flushable, Flush());

    // Calling hook directly to avoid aborting the test process.
    infra::ExecuteLogAndAbortHook("reason", "filename.cpp", 32, "speak %s and enter", "friend");

    EXPECT_THAT(stream.Storage(), testing::StrEq("\r\n\r\nreason! [speak friend and enter] at filename.cpp:32\r\n"));
}

TEST_F(LogAndAbortTracerTest, log_and_abort_with_tracer_provider_calls_tracer)
{
    services::LogAndAbortTracer logAndAbortTracer([this]() -> services::Tracer&
        {
            return tracer;
        });

    // Calling hook directly to avoid aborting the test process.
    infra::ExecuteLogAndAbortHook("reason", "filename.cpp", 32, "speak %s and enter", "friend");

    EXPECT_THAT(stream.Storage(), testing::StrEq("\r\n\r\nreason! [speak friend and enter] at filename.cpp:32\r\n"));
}
