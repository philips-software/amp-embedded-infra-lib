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

        void Commit();
        void Undo();

    private:
        OutputStream& stream;
    };

    class SavedMarkerTextStream
        : public TextOutputStream::WithWriter<SavedMarkerStreamWriter>
    {
    public:
        SavedMarkerTextStream(OutputStream& stream, std::size_t marker);
        SavedMarkerTextStream(OutputStream& stream, std::size_t marker, const SoftFail&);
        SavedMarkerTextStream(OutputStream& stream, std::size_t marker, const NoFail&);
    };

    class SavedMarkerDataStream
        : public DataOutputStream::WithWriter<SavedMarkerStreamWriter>
    {
    public:
        SavedMarkerDataStream(OutputStream& stream, std::size_t marker);
        SavedMarkerDataStream(OutputStream& stream, std::size_t marker, const SoftFail&);
        SavedMarkerDataStream(OutputStream& stream, std::size_t marker, const NoFail&);
    };
}

#endif
