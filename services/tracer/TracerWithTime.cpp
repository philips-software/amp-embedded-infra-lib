#include "infra/stream/StreamManipulators.hpp"
#include "infra/timer/Timer.hpp"
#include "services/tracer/TracerWithTime.hpp"

namespace services
{
    TracerWithTime::TracerWithTime(infra::TextOutputStream& stream)
        : Tracer(stream)
    {}

    void TracerWithTime::InsertHeader()
    {
        infra::TimePoint nowTimePoint = infra::Now();
        time_t now = std::chrono::system_clock::to_time_t(nowTimePoint);
        std::tm* utcTime = gmtime(&now);
        assert(utcTime != nullptr);

        Continue() << utcTime->tm_hour << ':' << utcTime->tm_min << ':' << utcTime->tm_sec  << '.'
            << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(nowTimePoint.time_since_epoch()).count() % 1000000 << ' ';
    }
}
