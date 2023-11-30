#ifndef INFRA_BUFFERING_STREAM_WRITER_HPP
#define INFRA_BUFFERING_STREAM_WRITER_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace infra
{
    // Usage: Any data that does not fit into the output stream is written to the buffer
    // Any data already present in the buffer is written to the output stream upon construction of BufferingStreamWriter
    class BufferingStreamWriter
        : public infra::StreamWriter
    {
    public:
        BufferingStreamWriter(infra::BoundedDeque<uint8_t>& buffer, infra::StreamWriter& output);

        // Implementation of StreamWriter
        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        [[noreturn]] infra::ByteRange SaveState(std::size_t marker) override;
        [[noreturn]] void RestoreState(infra::ByteRange range) override;
        [[noreturn]] infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        void LoadRemainder();

    private:
        infra::BoundedDeque<uint8_t>& buffer;
        infra::StreamWriter& output;
        std::size_t index = 0;
    };
}

#endif
