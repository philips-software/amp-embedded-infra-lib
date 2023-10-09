#ifndef INFRA_LIMITED_OUTPUT_STREAM_HPP
#define INFRA_LIMITED_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"

namespace infra
{
    class LimitedStreamWriter
        : public StreamWriter
    {
    public:
        template<class T>
        using WithOutput = infra::WithStorage<LimitedStreamWriter, T>;

        LimitedStreamWriter(StreamWriter& output, uint32_t length);
        LimitedStreamWriter(const LimitedStreamWriter& other);
        ~LimitedStreamWriter() = default;

    public:
        void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        std::size_t GetProcessedBytesSince(std::size_t marker) const override;
        infra::ByteRange SaveState(std::size_t marker) override;
        void RestoreState(infra::ByteRange range) override;
        infra::ByteRange Overwrite(std::size_t marker) override;

    private:
        StreamWriter& output;
        uint32_t length;
    };

    class LimitedTextOutputStream
        : public TextOutputStream::WithWriter<LimitedStreamWriter>
    {
    public:
        using TextOutputStream::WithWriter<LimitedStreamWriter>::WithWriter;
    };

    class LimitedDataOutputStream
        : public DataOutputStream::WithWriter<LimitedStreamWriter>
    {
    public:
        using DataOutputStream::WithWriter<LimitedStreamWriter>::WithWriter;
    };
}

#endif
