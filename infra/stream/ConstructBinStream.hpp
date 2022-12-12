#ifndef INFRA_CONSTRUCT_BIN_STREAM_HPP
#define INFRA_CONSTRUCT_BIN_STREAM_HPP

#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"

namespace infra
{
    struct TextStreamHelper
    {
        std::function<void(infra::TextOutputStream stream)> callback;
    };

    TextStreamHelper TextStream(const std::function<void(infra::TextOutputStream stream)>& callback);

    ConstructBin& operator<<(ConstructBin& constructBin, const TextStreamHelper& helper);
    ConstructBin& operator<<(ConstructBin&& constructBin, const TextStreamHelper& helper);
}

#endif
