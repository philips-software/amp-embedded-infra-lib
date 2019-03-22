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
}
