#ifndef INFRA_COUNTING_OUTPUT_STREAM_HPP
#define INFRA_COUNTING_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"

namespace infra
{
    class CountingStreamWriter
        : public StreamWriter
    {
    public:
        CountingStreamWriter(infra::ByteRange saveStateStorage = {});

        std::size_t Processed() const;

        void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        infra::ByteRange SaveState(std::size_t marker) override;
        void RestoreState(infra::ByteRange range) override;
        infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        infra::ByteRange saveStateStorage;
        std::size_t processed = 0;
    };
}

#endif
