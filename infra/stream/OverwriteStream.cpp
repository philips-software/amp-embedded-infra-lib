#include "infra/stream/OverwriteStream.hpp"

namespace infra
{
    OverwriteStreamWriter::OverwriteStreamWriter(OutputStream& stream, std::size_t marker)
        : ByteOutputStreamWriter(stream.Writer().Overwrite(marker))
    {}
}
