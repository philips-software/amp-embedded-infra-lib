#ifndef INFRA_SAVED_MARKER_STREAM_HPP
#define INFRA_SAVED_MARKER_STREAM_HPP

#include "infra/stream/ByteOutputStream.hpp"

namespace infra
{
    class SavedMarkerStreamWriter
        : public ByteOutputStreamWriter
    {
    public:
        SavedMarkerStreamWriter(OutputStream& stream, std::size_t marker);
        ~SavedMarkerStreamWriter();

    private:
        OutputStream& stream;
    };

    class SavedMarkerTextStream
        : public TextOutputStream::WithWriter<SavedMarkerStreamWriter>
    {
    public:
        using TextOutputStream::WithWriter<SavedMarkerStreamWriter>::WithWriter;
    };

    class SavedMarkerDataStream
        : public DataOutputStream::WithWriter<SavedMarkerStreamWriter>
    {
    public:
        using DataOutputStream::WithWriter<SavedMarkerStreamWriter>::WithWriter;
    };
}

#endif
