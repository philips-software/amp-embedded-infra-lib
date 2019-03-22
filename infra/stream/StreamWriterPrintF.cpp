#include "infra/stream/StreamWriterPrintF.hpp"
#include <cstdio>

namespace infra
{
    void StreamWriterPrintF::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {
        std::array<uint8_t, 128> printBuffer;

        while (!range.empty())
        {
            auto from = infra::Head(range, printBuffer.size() - 1);
            auto to = infra::Head(infra::ByteRange(printBuffer), from.size());
            infra::Copy(from, to);
            range.pop_front(from.size());

            printBuffer[to.size()] = 0;
            printf("%s", printBuffer.data());
        }
    }

    std::size_t StreamWriterPrintF::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }
}
