#include "services/tracer/StreamWriterOnSeggerRtt.hpp"
#include "RTT/SEGGER_RTT.h"

namespace services
{
    StreamWriterOnSeggerRtt::StreamWriterOnSeggerRtt(unsigned int bufferIndex)
        : bufferIndex(bufferIndex)
    {}

    void StreamWriterOnSeggerRtt::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        range.shrink_from_back_to(Available());
        SEGGER_RTT_Write(bufferIndex, range.cbegin(), range.size());
    }

    std::size_t StreamWriterOnSeggerRtt::Available() const
    {
        return SEGGER_RTT_GetAvailWriteSpace(bufferIndex);
    }
}
