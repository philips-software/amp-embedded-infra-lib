#ifndef INFRA_OVERWRITE_STREAM_HPP
#define INFRA_OVERWRITE_STREAM_HPP

#include "infra/stream/ByteOutputStream.hpp"

namespace infra
{
    class OverwriteStreamWriter
        : public ByteOutputStreamWriter
    {
    public:
        OverwriteStreamWriter(OutputStream& stream, std::size_t marker);
    };

    class OverwriteTextStream
        : public TextOutputStream::WithWriter<OverwriteStreamWriter>
    {
    public:
        using TextOutputStream::WithWriter<OverwriteStreamWriter>::WithWriter;
    };

    class OverwriteDataStream
        : public DataOutputStream::WithWriter<OverwriteStreamWriter>
    {
    public:
        using DataOutputStream::WithWriter<OverwriteStreamWriter>::WithWriter;
    };
}

#endif
