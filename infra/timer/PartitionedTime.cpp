#include "infra/timer/PartitionedTime.hpp"

namespace infra
{
    PartitionedTime::PartitionedTime(time_t unixTime)
    {
        int z = static_cast<int>((unixTime >= 0) ? (unixTime / (24 * 60 * 60)) : ((unixTime - (24 * 60 * 60)) / (24 * 60 * 60)));
        unixTime -= z * (24 * 60 * 60);

        seconds = unixTime % 60;
        unixTime = unixTime / 60;

        minutes = unixTime % 60;
        unixTime = unixTime / 60;

        hours = unixTime % 24;
        unixTime = unixTime / 24;

        // See http://howardhinnant.github.io/date_algorithms.html for this wonderful algorithm
        z += 719468;
        const unsigned era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = static_cast<unsigned>(z - era * 146097);               // [0, 146096]
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; // [0, 399]
        const int y = static_cast<int>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
        const unsigned mp = (5 * doy + 2) / 153;                      // [0, 11]
        const unsigned d = doy - (153 * mp + 2) / 5 + 1;              // [1, 31]
        const unsigned m = mp + (mp < 10 ? 3 : -9);                   // [1, 12]

        years = y + (m <= 2);
        months = m;
        days = d;
    };
}