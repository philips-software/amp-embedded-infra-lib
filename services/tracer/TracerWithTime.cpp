#include "services/tracer/TracerWithTime.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/timer/PartitionedTime.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    TracerWithTime::TracerWithTime(infra::TextOutputStream& stream)
        : TracerToStream(stream)
    {}

    void TracerWithTime::InsertHeader()
    {
        auto now = infra::Now();
        infra::PartitionedTime partitioned(now);

        Continue() << infra::Width(2, '0') << partitioned.hours << infra::resetWidth << ':'
                   << infra::Width(2, '0') << partitioned.minutes << infra::resetWidth << ':'
                   << infra::Width(2, '0') << partitioned.seconds << infra::resetWidth << '.'
                   << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000 << ' ';
    }
}
