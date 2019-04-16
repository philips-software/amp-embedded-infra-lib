#include "infra/timer/StreamTime.hpp"

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, infra::TimePoint time)
    {
        time_t now = std::chrono::system_clock::to_time_t(time);
        std::tm* utcTime = gmtime(&now);
        assert(utcTime != nullptr);

        const auto w02 = infra::Width(2, '0');
        stream << (utcTime->tm_year + 1900) << '-'
            << w02 << (utcTime->tm_mon + 1) << infra::resetWidth << '-'
            << w02 << utcTime->tm_mday << infra::resetWidth << ' '
            << w02 << utcTime->tm_hour << infra::resetWidth << ':'
            << w02 << utcTime->tm_min << infra::resetWidth << ':'
            << w02 << utcTime->tm_sec << infra::resetWidth << '.'
            << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() % 1000000;

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream&& stream, infra::TimePoint time)
    {
        return stream << time;
    }
}
