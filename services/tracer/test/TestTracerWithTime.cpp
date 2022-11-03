#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/tracer/TracerWithTime.hpp"
#include "gmock/gmock.h"

class TracerWithTimeTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TracerWithTimeTest, trace_inserts_time)
{
    ForwardTime(std::chrono::hours(10) + std::chrono::minutes(20) + std::chrono::seconds(30));

    infra::StringOutputStream::WithStorage<32> stream;
    services::TracerWithTime tracer(stream);

    tracer.Trace();
    EXPECT_EQ("\r\n10:20:30.000000 ", stream.Storage());
}
