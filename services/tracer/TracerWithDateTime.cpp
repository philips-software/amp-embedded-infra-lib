#include "infra/stream/StreamManipulators.hpp"
#include "infra/timer/StreamTime.hpp"
#include "services/tracer/TracerWithDateTime.hpp"

namespace services
{
    TracerWithDateTime::TracerWithDateTime(infra::TextOutputStream& stream, const infra::TimerService& timerService)
        : Tracer(stream)
        , timerService(timerService)
    {}

    void TracerWithDateTime::InsertHeader()
    {
        Continue() << timerService.Now() << ' ';
    }
}
