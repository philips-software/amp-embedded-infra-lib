#ifndef INFRA_STREAM_WRITER_FILE_HPP
#define INFRA_STREAM_WRITER_FILE_HPP

#include "infra/stream/OutputStream.hpp"
#include <cstdio>

namespace infra
{
    class StreamWriterFile
        : public StreamWriter
    {
    public:
        explicit StreamWriterFile(const char* path);
        ~StreamWriterFile();

        void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;

    private:
        FILE* stream;
    };
}

#endif
