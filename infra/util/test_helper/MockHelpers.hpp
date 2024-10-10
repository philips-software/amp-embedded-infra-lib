#ifndef INFRA_MOCK_HELPERS_HPP
#define INFRA_MOCK_HELPERS_HPP

#include "gmock/gmock.h"
#include <vector>

namespace infra
{
    MATCHER_P(CheckByteRangeContents, contents, "")
    {
        return std::vector<uint8_t>(arg.begin(), arg.end()) == std::vector<uint8_t>(contents.begin(), contents.end());
    }

    ACTION_TEMPLATE(SaveRef,
        HAS_1_TEMPLATE_PARAMS(int, k),
        AND_1_VALUE_PARAMS(pointer))
    {
        *pointer = &::std::get<k>(args);
    }
}

#endif
