#include "infra/stream/StdStringOutputStream.hpp"

namespace infra
{
    StdStringOutputStreamWriter::StdStringOutputStreamWriter(std::string& string)
        : string(string)
    {}

    void StdStringOutputStreamWriter::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        string.append(reinterpret_cast<const char*>(range.begin()), range.size());
    }

    std::size_t StdStringOutputStreamWriter::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }
}
