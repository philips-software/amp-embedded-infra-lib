#include "gmock/gmock.h"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/TracerWithDateTime.hpp"

class TracerWithDateTimeTestImpl
    : public services::TracerWithDateTime
{
public:
    TracerWithDateTimeTestImpl(infra::TimerService& timerService)
        : services::TracerWithDateTime(stream, timerService)
    {}

    infra::TextOutputStream Continue()
    {
        return stream;
    }

    infra::StringOutputStream::WithStorage<32> stream;
};

class TracerWithDateTimeTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TracerWithDateTimeTest, trace_inserts_date_time)
{
    tm timePoint = {};
    timePoint.tm_sec = 14;
    timePoint.tm_min = 35;
    timePoint.tm_hour = 16;
    timePoint.tm_mday = 12;
    timePoint.tm_mon = 6;
    timePoint.tm_year = 116;

    time_t nullTime = 100000;
    std::tm* localUtcDiff = gmtime(&nullTime);
    assert(localUtcDiff != nullptr);
    time_t diff = mktime(localUtcDiff);

    ForwardTime(std::chrono::seconds(mktime(&timePoint) + nullTime - diff) + std::chrono::microseconds(1));
    TracerWithDateTimeTestImpl tracer(systemTimerService);

    tracer.Trace();
    EXPECT_EQ("\r\n2016-07-12 16:35:14.000001 ", tracer.stream.Storage());
}
