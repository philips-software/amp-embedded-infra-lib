#ifndef INFRA_STREAM_FILESYSTEM_STREAMING_HPP
#define INFRA_STREAM_FILESYSTEM_STREAMING_HPP

#include "infra/stream/OutputStream.hpp"
#include <filesystem>

namespace infra
{
    infra::TextOutputStream operator<<(TextOutputStream stream, const std::filesystem::path& path)
    {
        return stream << path.string();
    }
}

#endif
