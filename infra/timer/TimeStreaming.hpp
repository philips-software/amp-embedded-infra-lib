#ifndef INFRA_TIME_STREAMING_HPP
#define INFRA_TIME_STREAMING_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/timer/Timer.hpp"

namespace infra
{
    infra::TextOutputStream operator<<(TextOutputStream stream, const TimePoint& timePoint);
    TextOutputStream operator<<(TextOutputStream stream, const Duration& duration);
}

#endif
