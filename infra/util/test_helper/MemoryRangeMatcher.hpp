#ifndef INFRA_MEMORY_RANGE_MATCHER_HPP
#define INFRA_MEMORY_RANGE_MATCHER_HPP

#include "gmock/gmock.h"
#include "infra/util/MemoryRange.hpp"

namespace infra
{
    MATCHER_P(ContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return infra::ContentsEqual(infra::MakeRange(x), arg);
    }
}

#endif
