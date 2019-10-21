#ifndef INFRA_COUNTING_OUTPUT_STREAM_HPP
#define INFRA_COUNTING_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"

namespace infra
{
    class CountingStreamWriter
        : public StreamWriter
    {
    public:
        std::size_t Processed() const;

        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;
        virtual std::size_t ConstructSaveMarker() const override;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        virtual infra::ByteRange SaveState(std::size_t marker) override;
        virtual void RestoreState(infra::ByteRange range) override;
        virtual infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        std::size_t processed = 0;
    };
}

#endif
