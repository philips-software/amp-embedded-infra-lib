#include "infra/timer/StreamTime.hpp"

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, infra::TimePoint time)
    {
        time_t now = std::chrono::system_clock::to_time_t(time);
        std::tm* utcTime = gmtime(&now);
        assert(utcTime != nullptr);

        stream << (utcTime->tm_year + 1900) << '-' << infra::Width(2, '0') << (utcTime->tm_mon + 1) << '-' << utcTime->tm_mday << ' '
            << utcTime->tm_hour << ':' << utcTime->tm_min << ':' << utcTime->tm_sec  << '.'
            << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() % 1000000;

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream&& stream, infra::TimePoint time)
    {
        return stream << time;
    }
}
