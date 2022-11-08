#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/tracer/TracerWithDateTime.hpp"
#include "gmock/gmock.h"

class TracerWithDateTimeTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TracerWithDateTimeTest, trace_inserts_date_time)
{
    ForwardTime(std::chrono::hours(1) + std::chrono::minutes(20) + std::chrono::seconds(30) + std::chrono::microseconds(1));

    infra::StringOutputStream::WithStorage<32> stream;
    services::TracerWithDateTime tracer(stream);

    tracer.Trace();
    EXPECT_EQ("\r\n1970-01-01T01:20:30.000001 ", stream.Storage());
}
