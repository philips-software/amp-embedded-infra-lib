#ifndef INFRA_STREAM_TIME_HPP
#define INFRA_STREAM_TIME_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/timer/Timer.hpp"

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, infra::TimePoint time);
    TextOutputStream& operator<<(TextOutputStream&& stream, infra::TimePoint time);
}

#endif
