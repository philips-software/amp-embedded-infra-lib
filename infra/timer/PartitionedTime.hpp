#ifndef INFRA_TIMER_PARTITIONED_TIME_HPP
#define INFRA_TIMER_PARTITIONED_TIME_HPP

#include <cstdint>
#include <time.h>

namespace infra
{
    struct PartitionedTime
    {
        PartitionedTime(time_t unixTime);       

        uint8_t seconds; // seconds after the minute - [0, 59]
        uint8_t minutes; // minutes after the hour - [0, 59]
        uint8_t hours;   // hours since midnight - [0, 23]
        uint8_t days;    // day of the month - [1, 31]
        uint8_t months;  // months since January - [1, 12]
        uint16_t years;  // years since 1900
    };
}
#endif
