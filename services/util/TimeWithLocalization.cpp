#include "services/util/TimeWithLocalization.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/stream/StreamManipulators.hpp"
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

    tm* TimeWithLocalization::GetTm(infra::TimePoint timePoint)
    {
        auto timeAsTimeType = std::chrono::system_clock::to_time_t(timePoint);
        auto timeAsGmTime = std::gmtime(&timeAsTimeType);
        assert(timeAsGmTime != nullptr);
        return timeAsGmTime;
    }

    infra::Optional<infra::TimePoint> TimeWithLocalization::TimePointFromString(infra::BoundedConstString timePointString)
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
            return infra::none;

        return infra::MakeOptional(infra::PartitionedTime(year, month, day, hour, minute, second).ToTimePoint());
    }

    infra::Optional<infra::Duration> TimeWithLocalization::DurationFromString(infra::BoundedConstString durationString)
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
            return infra::none;

        if (sign == '-')
        {
            hour = -hour;
            minute = -minute;
        }

        return infra::Optional<infra::Duration>(infra::inPlace, std::chrono::hours(hour) + std::chrono::minutes(minute));
    }

    infra::Optional<infra::BoundedConstString> TimeWithLocalization::OffsetFromTimeString(infra::BoundedConstString timePointString)
    {
        if (timePointString.size() <= DateTimeSize)
            return infra::none;

        infra::BoundedConstString left = timePointString.substr(DateTimeSize);

        if (left.find_first_of('Z') != infra::BoundedString::npos && left.find_first_of('Z') == left.size() - 1)
            return infra::Optional<infra::BoundedConstString>(infra::inPlace, "+00:00");
        else if (left.find('+') != infra::BoundedString::npos)
            return infra::Optional<infra::BoundedConstString>(infra::inPlace, left.substr(left.find_first_of('+')));
        else if (left.find('-') != infra::BoundedString::npos)
            return infra::Optional<infra::BoundedConstString>(infra::inPlace, left.substr(left.find_first_of('-')));

        return infra::none;
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

infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const infra::TimePoint& timePoint)
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

infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const infra::Duration& duration)
{
    const auto isNegative = duration < infra::Duration::zero();
    const auto d = isNegative ? -duration : duration;
    const auto w02 = infra::Width(2, '0');
    stream << (isNegative ? '-' : '+')  << w02 << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::hours>(d).count())
           << infra::resetWidth << ":"  << w02 << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::minutes>(d).count() % 60);
    return stream;
}
