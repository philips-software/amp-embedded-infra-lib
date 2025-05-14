#ifndef INFRA_STREAM_WRITER_FILE_HPP
#define INFRA_STREAM_WRITER_FILE_HPP

#include "infra/stream/OutputStream.hpp"
#include <cstdio>
#include <fstream>

namespace infra
{
    class StreamWriterFile
        : public StreamWriter
    {
    public:
        explicit StreamWriterFile(const char* path);

        void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;

    private:
        std::ofstream stream;
    };
}

#endif
