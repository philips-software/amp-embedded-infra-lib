#include "services/tracer/TracerWithDateTime.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/timer/PartitionedTime.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    TracerWithDateTime::TracerWithDateTime(infra::TextOutputStream& stream)
        : Tracer(stream)
    {}

    void TracerWithDateTime::InsertHeader()
    {
        auto now = infra::Now();
        auto utcTime = infra::PartitionedTime(std::chrono::system_clock::to_time_t(now));

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
