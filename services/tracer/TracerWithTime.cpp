#include "services/tracer/TracerWithTime.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/timer/PartitionedTime.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    TracerWithTime::TracerWithTime(infra::TextOutputStream& stream)
        : Tracer(stream)
    {}

    void TracerWithTime::InsertHeader()
    {
        auto nowTimePoint = infra::Now();
        infra::PartitionedTime partitioned(nowTimePoint);

        Continue() << partitioned.hours << ':' << partitioned.minutes << ':' << partitioned.seconds << '.'
                   << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(nowTimePoint.time_since_epoch()).count() % 1000000 << ' ';
    }
}
