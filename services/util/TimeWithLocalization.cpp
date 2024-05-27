#include "services/util/TimeWithLocalization.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/PartitionedTime.hpp"

namespace services
{
    TimeWithLocalization::TimeWithLocalization(uint32_t systemTimerServiceId, uint32_t utcTimerServiceId, uint32_t localTimeWithoutDaylightSavingTimerServiceId, uint32_t localTimeTimerServiceId)
        : utcTimerService(utcTimerServiceId, infra::TimerService::GetTimerService(systemTimerServiceId))
        , localTimeWithoutDaylightSavingTimerService(localTimeWithoutDaylightSavingTimerServiceId, utcTimerService)
        , localTimeTimerService(localTimeTimerServiceId, localTimeWithoutDaylightSavingTimerService)
    {}

    infra::DerivedTimerService& TimeWithLocalization::Utc()
    {
        return utcTimerService;
    }

    infra::DerivedTimerService& TimeWithLocalization::LocalWithoutDaylightSaving()
    {
        return localTimeWithoutDaylightSavingTimerService;
    }

    infra::DerivedTimerService& TimeWithLocalization::Local()
    {
        return localTimeTimerService;
    }

    std::optional<infra::TimePoint> TimeWithLocalization::TimePointFromString(infra::BoundedConstString timePointString)
    {
        uint16_t year = 0;
        uint8_t month = 0;
        uint8_t day = 0;
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
        char separator = {};

        infra::StringInputStream stream(timePointString, infra::softFail);
        stream >> year >> separator >> month >> separator >> day >> separator >> hour >> separator >> minute >> separator >> second;

        if (stream.Failed())
            return std::nullopt;

        return std::make_optional(infra::PartitionedTime(year, month, day, hour, minute, second).ToTimePoint());
    }

    std::optional<infra::Duration> TimeWithLocalization::DurationFromString(infra::BoundedConstString durationString)
    {
        char sign = {};
        char colon = {};
        int8_t hour = 0;
        int8_t minute = 0;

        infra::StringInputStream stream(durationString, infra::softFail);
        stream >> sign >> hour >> colon >> minute;

        if (stream.Failed() ||
            colon != ':' ||
            (sign != '-' && sign != '+'))
            return std::nullopt;

        if (sign == '-')
        {
            hour = -hour;
            minute = -minute;
        }

        return std::optional<infra::Duration>(std::in_place, std::chrono::hours(hour) + std::chrono::minutes(minute));
    }

    std::optional<infra::BoundedConstString> TimeWithLocalization::OffsetFromTimeString(infra::BoundedConstString timePointString)
    {
        if (timePointString.size() <= DateTimeSize)
            return std::nullopt;

        infra::BoundedConstString left = timePointString.substr(DateTimeSize);

        if (left.find_first_of('Z') != infra::BoundedString::npos && left.find_first_of('Z') == left.size() - 1)
            return std::optional<infra::BoundedConstString>(std::in_place, "+00:00");
        else if (left.find('+') != infra::BoundedString::npos)
            return std::optional<infra::BoundedConstString>(std::in_place, left.substr(left.find_first_of('+')));
        else if (left.find('-') != infra::BoundedString::npos)
            return std::optional<infra::BoundedConstString>(std::in_place, left.substr(left.find_first_of('-')));

        return std::nullopt;
    }

    infra::Duration TimeWithLocalization::GetOffsetFromLocalToUtc() const
    {
        return localTimeWithoutDaylightSavingTimerService.GetCurrentShift() + localTimeTimerService.GetCurrentShift();
    }

    infra::Duration TimeWithLocalization::GetOffsetDaylightSaving() const
    {
        return localTimeTimerService.GetCurrentShift();
    }

    infra::Duration TimeWithLocalization::GetOffsetTimezone() const
    {
        return localTimeWithoutDaylightSavingTimerService.GetCurrentShift();
    }
}

infra::TextOutputStream operator<<(infra::TextOutputStream stream, const infra::TimePoint& timePoint)
{
    auto time = infra::PartitionedTime(std::chrono::system_clock::to_time_t(timePoint));

    const auto w02 = infra::Width(2, '0');
    stream << time.years << '-'
           << w02 << time.months << infra::resetWidth << '-'
           << w02 << time.days << infra::resetWidth << 'T'
           << w02 << time.hours << infra::resetWidth << ':'
           << w02 << time.minutes << infra::resetWidth << ':'
           << w02 << time.seconds;

    return stream;
}
