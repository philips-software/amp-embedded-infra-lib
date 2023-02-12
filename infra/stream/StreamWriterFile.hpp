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

        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

    private:
        FILE* stream;
    };
}

#endif
