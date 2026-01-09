#include "services/tracer/LogAndAbortTracer.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace
{
    struct StdOutTracer
    {
        // Basically TracerOnIoOutputInfrastructure, but does not set global tracer

        infra::IoOutputStreamWriter writer;
        infra::TextOutputStream::WithErrorPolicy stream{ writer };
        services::TracerWithDateTime tracer{ stream };
    };

    class LogAndAbortTracerTest
        : public testing::Test
    {
    public:
        StdOutTracer tracer;
    };
}

TEST_F(LogAndAbortTracerTest, log_and_abort_without_registered_tracer_doesnt_call_tracer)
{
    testing::internal::CaptureStdout();
    // Calling hook directly to avoid aborting the test process.
    infra::ExecuteLogAndAbortHook("speak %s and enter", "elf");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output.empty(), true);
}

TEST_F(LogAndAbortTracerTest, log_and_abort_with_registered_tracer_calls_tracer)
{
    services::RegisterLogAndAbortTracerProvider([this]() -> services::Tracer&
        {
            return tracer.tracer;
        });

    testing::internal::CaptureStdout();
    // Calling hook directly to avoid aborting the test process.
    infra::ExecuteLogAndAbortHook("speak %s and enter", "friend");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("speak friend and enter"));
}
