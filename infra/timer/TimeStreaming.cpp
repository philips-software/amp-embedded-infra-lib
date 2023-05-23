#include "infra/timer/PartitionedTime.hpp"
#include "infra/timer/TimeStreaming.hpp"

namespace infra
{
    TextOutputStream operator<<(TextOutputStream stream, const TimePoint& timePoint)
    {
        auto time = PartitionedTime(std::chrono::system_clock::to_time_t(timePoint));

        const auto w02 = Width(2, '0');
        stream << time.years << '-'
            << w02 << time.months << resetWidth << '-'
            << w02 << time.days << resetWidth << 'T'
            << w02 << time.hours << resetWidth << ':'
            << w02 << time.minutes << resetWidth << ':'
            << w02 << time.seconds;

        return stream;
    }

    TextOutputStream operator<<(TextOutputStream stream, const Duration& duration)
    {
        const auto isNegative = duration < Duration::zero();
        const auto d = isNegative ? -duration : duration;
        const auto w02 = Width(2, '0');
        stream << (isNegative ? '-' : '+') << w02 << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::hours>(d).count())
            << resetWidth << ":" << w02 << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::minutes>(d).count() % 60);
        return stream;
    }
}
