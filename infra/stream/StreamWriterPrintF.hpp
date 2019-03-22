#ifndef INFRA_STREAM_WRITER_ON_PRINTF_HPP
#define INFRA_STREAM_WRITER_ON_PRINTF_HPP

#include "infra/stream/OutputStream.hpp"

namespace infra
{
    class StreamWriterPrintF
        : public StreamWriter
    {
    public:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;
    };
}

#endif
