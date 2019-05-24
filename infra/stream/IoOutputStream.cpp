#include "infra/stream/IoOutputStream.hpp"
#include <cstdlib>
#include <limits>

namespace infra
{
    IoOutputStreamWriter::IoOutputStreamWriter(std::ostream& stream)
        : stream(stream)
    {}

    void IoOutputStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        stream << std::string(range.begin(), range.end()) << std::flush;
    }

    std::size_t IoOutputStreamWriter::Available() const
    {
        return std::numeric_limits<std::size_t>::max();
    }
}
