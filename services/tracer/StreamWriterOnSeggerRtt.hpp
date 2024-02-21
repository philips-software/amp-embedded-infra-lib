#ifndef SERVICES_STREAM_WRITER_ON_SEGGER_RTT_HPP
#define SERVICES_STREAM_WRITER_ON_SEGGER_RTT_HPP

#include "infra/stream/OutputStream.hpp"

namespace services
{
    class StreamWriterOnSeggerRtt
        : public infra::StreamWriter
    {
    public:
        explicit StreamWriterOnSeggerRtt(unsigned int bufferIndex = 0);

        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;

    private:
        unsigned int bufferIndex;
    };
}

#endif
