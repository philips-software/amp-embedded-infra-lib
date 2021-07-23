#ifndef SERVICES_TIME_WITH_LOCALIZATION_HPP
#define SERVICES_TIME_WITH_LOCALIZATION_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/timer/DerivedTimerService.hpp"
#include "infra/util/Optional.hpp"

namespace services
{
    class TimeWithLocalization
    {
    public:
        static const std::size_t DateTimeSize = 19;
        static const std::size_t TimeOffsetSize = 6;

    public:
        explicit TimeWithLocalization(uint32_t systemTimerServiceId = infra::systemTimerServiceId, uint32_t utcTimerServiceId = 1, uint32_t localTimeWithoutDaylightSavingTimerServiceId = 2, uint32_t localTimeTimerServiceId = 3);

        infra::DerivedTimerService& Utc();
        infra::DerivedTimerService& LocalWithoutDaylightSaving();
        infra::DerivedTimerService& Local();

        static infra::Optional<infra::TimePoint> TimePointFromString(infra::BoundedConstString timePointString);
        static infra::Optional<infra::Duration> DurationFromString(infra::BoundedConstString durationString);
        static infra::Optional<infra::BoundedConstString> OffsetFromTimeString(infra::BoundedConstString timePointString);

        infra::Duration GetOffsetFromLocalToUtc() const;
        infra::Duration GetOffsetTimezone() const;
        infra::Duration GetOffsetDaylightSaving() const;

        static tm* GetTm(infra::TimePoint timePoint);

    private:
        infra::DerivedTimerService utcTimerService;
        infra::DerivedTimerService localTimeWithoutDaylightSavingTimerService;
        infra::DerivedTimerService localTimeTimerService;
    };
}

infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const infra::TimePoint& timePoint);
infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const infra::Duration& duration);

#endif
