#include "infra/stream/ConstructBinStream.hpp"

namespace infra
{
    TextStreamHelper TextStream(const std::function<void(infra::TextOutputStream stream)>& callback)
    {
        return TextStreamHelper{ callback };
    }

    ConstructBin& operator<<(ConstructBin& constructBin, const TextStreamHelper& helper)
    {
        helper.callback(infra::StdVectorOutputStream(constructBin.Vector()) << infra::text);
        return constructBin;
    }

    ConstructBin& operator<<(ConstructBin&& constructBin, const TextStreamHelper& helper)
    {
        helper.callback(infra::StdVectorOutputStream(constructBin.Vector()) << infra::text);
        return constructBin;
    }
}
