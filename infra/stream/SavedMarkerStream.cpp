#include "infra/stream/SavedMarkerStream.hpp"

namespace infra
{
    SavedMarkerStreamWriter::SavedMarkerStreamWriter(OutputStream& stream, std::size_t marker)
        : ByteOutputStreamWriter(stream.Writer().SaveState(marker))
        , stream(stream)
    {}

    SavedMarkerStreamWriter::~SavedMarkerStreamWriter()
    {
        stream.Writer().RestoreState(Remaining());
    }

    void SavedMarkerStreamWriter::Commit()
    {
        Reset(Remaining());
    }

    void SavedMarkerStreamWriter::Undo()
    {
        Reset();
    }

    SavedMarkerTextStream::SavedMarkerTextStream(OutputStream& stream, std::size_t marker)
        : TextOutputStream::WithWriter<SavedMarkerStreamWriter>(stream, marker)
    {}

    SavedMarkerTextStream::SavedMarkerTextStream(OutputStream& stream, std::size_t marker, const SoftFail&)
        : TextOutputStream::WithWriter<SavedMarkerStreamWriter>(stream, softFail, marker)
    {}

    SavedMarkerTextStream::SavedMarkerTextStream(OutputStream& stream, std::size_t marker, const NoFail&)
        : TextOutputStream::WithWriter<SavedMarkerStreamWriter>(stream, noFail, marker)
    {}

    SavedMarkerDataStream::SavedMarkerDataStream(OutputStream& stream, std::size_t marker)
        : DataOutputStream::WithWriter<SavedMarkerStreamWriter>(stream, marker)
    {}

    SavedMarkerDataStream::SavedMarkerDataStream(OutputStream& stream, std::size_t marker, const SoftFail&)
        : DataOutputStream::WithWriter<SavedMarkerStreamWriter>(stream, softFail, marker)
    {}

    SavedMarkerDataStream::SavedMarkerDataStream(OutputStream& stream, std::size_t marker, const NoFail&)
        : DataOutputStream::WithWriter<SavedMarkerStreamWriter>(stream, noFail, marker)
    {}
}
