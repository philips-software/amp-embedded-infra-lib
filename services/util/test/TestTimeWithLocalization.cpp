#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/PartitionedTime.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/TimeWithLocalization.hpp"
#include "gmock/gmock.h"

class TimeWithLocalizationTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    TimeWithLocalizationTest()
    {}

    void CheckTimePoint(infra::BoundedConstString expected, infra::TimePoint timePoint)
    {
        infra::StringOutputStream::WithStorage<64> stream;
        stream << timePoint;
        EXPECT_EQ(expected, stream.Storage());
    }

    void CheckDuration(infra::BoundedConstString expected, infra::Duration duration)
    {
        infra::StringOutputStream::WithStorage<64> stream;
        stream << duration;
        EXPECT_EQ(expected, stream.Storage());
    }

    void CheckValidDuration(infra::Duration expected, infra::BoundedConstString string)
    {
        auto duration = services::TimeWithLocalization::DurationFromString(string);
        EXPECT_NE(std::nullopt, duration);
        EXPECT_EQ(expected, *duration);
    }

    void CheckInvalidDuration(infra::BoundedConstString string)
    {
        auto duration = services::TimeWithLocalization::DurationFromString(string);
        EXPECT_EQ(std::nullopt, duration);
    }

    void CheckValidTimePoint(infra::TimePoint expected, infra::BoundedConstString string)
    {
        auto timePoint = services::TimeWithLocalization::TimePointFromString(string);
        EXPECT_NE(std::nullopt, timePoint);
        EXPECT_EQ(expected, *timePoint);
    }

    void CheckInvalidTimePoint(infra::BoundedConstString string)
    {
        auto timePoint = services::TimeWithLocalization::TimePointFromString(string);
        EXPECT_EQ(std::nullopt, timePoint);
    }

    services::TimeWithLocalization time;
};

TEST_F(TimeWithLocalizationTest, ConvertTimePointToString)
{
    CheckTimePoint("1970-01-01T00:00:00", infra::TimePoint());
    CheckTimePoint("1969-12-31T23:59:59", infra::TimePoint(std::chrono::seconds(-1)));
    CheckTimePoint("1971-02-03T04:05:06", infra::TimePoint(std::chrono::seconds(34401906)));
    CheckTimePoint("2070-01-01T00:00:00", infra::TimePoint(std::chrono::seconds(3155760000)));
}

TEST_F(TimeWithLocalizationTest, ConvertDurationToString)
{
    CheckDuration("+00:00", infra::Duration());
    CheckDuration("+00:00", infra::Duration(std::chrono::seconds(59)));
    CheckDuration("+00:01", infra::Duration(std::chrono::seconds(60)));
    CheckDuration("+01:01", infra::Duration(std::chrono::hours(1) + std::chrono::minutes(1)));
    CheckDuration("+62:01", infra::Duration(std::chrono::hours(61) + std::chrono::minutes(61)));
    CheckDuration("-01:01", infra::Duration(std::chrono::hours(-1) + std::chrono::minutes(-1)));
}

TEST_F(TimeWithLocalizationTest, ConvertValidTimePointFromString)
{
    CheckValidTimePoint(infra::TimePoint(), "1970-01-01T00:00:00");
    CheckValidTimePoint(infra::TimePoint(std::chrono::seconds(34401906)), "1971-02-03T04:05:06");
}

TEST_F(TimeWithLocalizationTest, ConvertInvalidTimePointFromString)
{
    CheckInvalidTimePoint("");
    CheckInvalidTimePoint("xxxx-01-01T00:00:00");
    CheckInvalidTimePoint("1970-01-01T00:00:xx");
}

TEST_F(TimeWithLocalizationTest, ConvertValidDurationFromString)
{
    CheckValidDuration(infra::Duration(), "+00:00");
    CheckValidDuration(infra::Duration(), "-00:00");
    CheckValidDuration(infra::Duration(std::chrono::hours(1) + std::chrono::minutes(10)), "+01:10");
    CheckValidDuration(infra::Duration(std::chrono::hours(-1) + std::chrono::minutes(-10)), "-01:10");
}

TEST_F(TimeWithLocalizationTest, ConvertInvalidDurationFromString)
{
    CheckInvalidDuration("");
    CheckInvalidDuration("+xx:00");
    CheckInvalidDuration("+00-00");
}

TEST_F(TimeWithLocalizationTest, GetOffsetFromTimeString)
{
    EXPECT_EQ("+00:00", services::TimeWithLocalization::OffsetFromTimeString("1970-01-01T00:00:00Z"));
    EXPECT_EQ("+59:59", services::TimeWithLocalization::OffsetFromTimeString("1970-01-01T00:00:00+59:59"));
    EXPECT_EQ("-01:01", services::TimeWithLocalization::OffsetFromTimeString("1970-01-01T00:00:00-01:01"));
    EXPECT_EQ("-00:00", services::TimeWithLocalization::OffsetFromTimeString("1970-01-01T00:00:00-00:00"));
    EXPECT_EQ(std::nullopt, services::TimeWithLocalization::OffsetFromTimeString("1970-01-01T00:00:00"));
}

TEST_F(TimeWithLocalizationTest, GetOffsetValues)
{
    time.LocalWithoutDaylightSaving().Shift(std::chrono::minutes(60));
    time.Local().Shift(std::chrono::minutes(120));

    EXPECT_EQ(std::chrono::minutes(60), time.GetOffsetTimezone());
    EXPECT_EQ(std::chrono::minutes(120), time.GetOffsetDaylightSaving());
    EXPECT_EQ(std::chrono::minutes(180), time.GetOffsetFromLocalToUtc());
}

TEST_F(TimeWithLocalizationTest, WeekDay)
{
    infra::PartitionedTime time(2021, 7, 23, 0, 0, 0);
    EXPECT_EQ(5, time.WeekDay());
}
