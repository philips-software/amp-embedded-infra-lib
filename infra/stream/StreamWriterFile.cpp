#include "infra/stream/StreamWriterFile.hpp"

namespace infra
{
    StreamWriterFile::StreamWriterFile(const char* path)
        : stream(fopen(path, "w"))
    {}

    StreamWriterFile::~StreamWriterFile()
    {
        fclose(stream);
    }

    void StreamWriterFile::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        fwrite(range.begin(), 1, range.size(), stream);
        fflush(stream);
    }

    std::size_t StreamWriterFile::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }
}
