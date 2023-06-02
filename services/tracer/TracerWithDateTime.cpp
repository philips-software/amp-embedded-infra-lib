#include "services/tracer/TracerWithDateTime.hpp"
#include "infra/timer/TimeStreaming.hpp"

namespace services
{
    TracerWithDateTime::TracerWithDateTime(infra::TextOutputStream& stream)
        : Tracer(stream)
    {}

    void TracerWithDateTime::InsertHeader()
    {
        auto now = infra::Now();

        Continue() << now << '.' << infra::Width(6, '0') << std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000 << infra::resetWidth << ' ';
    }
}
