#include "infra/stream/StreamManipulators.hpp"
#include "services/tracer/TracerWithDateTime.hpp"

namespace services
{
    TracerWithDateTime::TracerWithDateTime(infra::TextOutputStream& stream, const infra::TimerService& timerService)
        : Tracer(stream)
        , timerService(timerService)
    {}

    struct PartitionedTime
    {
        PartitionedTime(time_t unixTime)
        {
            seconds = unixTime % 60;
            unixTime = unixTime / 60;

            minutes = unixTime % 60;
            unixTime = unixTime / 60;

            hours = unixTime % 24;
            unixTime = unixTime / 24;

            // See http://howardhinnant.github.io/date_algorithms.html for this wonderful algorithm
            unsigned z = unixTime;
            z += 719468;
            const unsigned era = (z >= 0 ? z : z - 146096) / 146097;
            const unsigned doe = static_cast<unsigned>(z - era * 146097);                   // [0, 146096]
            const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;     // [0, 399]
            const unsigned y = static_cast<unsigned>(yoe) + era * 400;
            const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);                   // [0, 365]
            const unsigned mp = (5 * doy + 2) / 153;                                        // [0, 11]
            const unsigned d = doy - (153 * mp + 2) / 5 + 1;                                // [1, 31]
            const unsigned m = mp + (mp < 10 ? 3 : -9);                                     // [1, 12]

            years = y + (m <= 2);
            months = m;
            days = d;
        };

        uint8_t seconds;   // seconds after the minute - [0, 59]
        uint8_t minutes;   // minutes after the hour - [0, 59]
        uint8_t hours;  // hours since midnight - [0, 23]
        uint8_t days;  // day of the month - [1, 31]
        uint8_t months;   // months since January - [1, 12]
        uint16_t years;  // years since 1900
    };

    void TracerWithDateTime::InsertHeader()
    {
        auto now = infra::Now();
        auto utcTime = PartitionedTime(std::chrono::system_clock::to_time_t(now));

        const auto w02 = infra::Width(2, '0');
        Continue() << utcTime.years << '-'
            << w02 << utcTime.months << infra::resetWidth << '-'
            << w02 << utcTime.days << infra::resetWidth << ' '
            << w02 << utcTime.hours << infra::resetWidth << ':'
            << w02 << utcTime.minutes << infra::resetWidth << ':'
            << w02 << utcTime.seconds << infra::resetWidth << '.'
            << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000
            << infra::resetWidth << " ";
    }
}
