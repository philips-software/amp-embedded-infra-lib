#ifndef SERVICES_TIME_WITH_LOCALIZATION_HPP
#define SERVICES_TIME_WITH_LOCALIZATION_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/timer/DerivedTimerService.hpp"
#include "infra/timer/TimeStreaming.hpp"
#include "infra/util/Optional.hpp"

namespace infra
{
    const uint32_t utcTimerServiceId = 1;
    const uint32_t localTimeWithoutDaylightSavingTimerServiceId = 2;
    const uint32_t localTimeTimerServiceId = 3;
}

namespace services
{
    class TimeWithLocalization
    {
    public:
        static const std::size_t DateTimeSize = 19;
        static const std::size_t TimeOffsetSize = 6;

    public:
        explicit TimeWithLocalization(uint32_t systemTimerServiceId = infra::systemTimerServiceId,
            uint32_t utcTimerServiceId = infra::utcTimerServiceId,
            uint32_t localTimeWithoutDaylightSavingTimerServiceId = infra::localTimeWithoutDaylightSavingTimerServiceId,
            uint32_t localTimeTimerServiceId = infra::localTimeTimerServiceId);

        infra::DerivedTimerService& Utc();
        infra::DerivedTimerService& LocalWithoutDaylightSaving();
        infra::DerivedTimerService& Local();

        static infra::Optional<infra::TimePoint> TimePointFromString(infra::BoundedConstString timePointString);
        static infra::Optional<infra::Duration> DurationFromString(infra::BoundedConstString durationString);
        static infra::Optional<infra::BoundedConstString> OffsetFromTimeString(infra::BoundedConstString timePointString);

        infra::Duration GetOffsetFromLocalToUtc() const;
        infra::Duration GetOffsetTimezone() const;
        infra::Duration GetOffsetDaylightSaving() const;

    private:
        infra::DerivedTimerService utcTimerService;
        infra::DerivedTimerService localTimeWithoutDaylightSavingTimerService;
        infra::DerivedTimerService localTimeTimerService;
    };
}

#endif
