#include "infra/stream/StreamWriterFile.hpp"

namespace infra
{
    StreamWriterFile::StreamWriterFile(const char* path)
        : stream(path, std::ios::binary)
    {}

    void StreamWriterFile::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        stream.write(reinterpret_cast<const char*>(range.begin()), range.size());
        stream.flush();
        errorPolicy.ReportResult(!stream.fail());
    }

    std::size_t StreamWriterFile::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }
}
