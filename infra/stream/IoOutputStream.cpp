#include "infra/stream/IoOutputStream.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace infra
{
    void IoOutputStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        std::cout << std::string(range.begin(), range.end()) << std::flush;
    }

    std::size_t IoOutputStreamWriter::Available() const
    {
        return std::numeric_limits<std::size_t>::max();
    }
}

