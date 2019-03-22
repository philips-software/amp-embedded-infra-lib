#include "infra/stream/StreamManipulators.hpp"
#include <cassert>

namespace infra
{
    const Text text;
    const Hex hex;
    const Bin bin;
    const Data data;
    const Endl endl;

    Width::Width(std::size_t width, char padding)
        : width(width)
        , padding(padding)
    {}

    const Width resetWidth(0);
}
