#ifndef INFRA_LIMITED_OUTPUT_STREAM_HPP
#define INFRA_LIMITED_OUTPUT_STREAM_HPP

#include "infra/stream/OutputStream.hpp"

namespace infra
{
    class LimitedStreamWriter
        : public StreamWriter
    {
    public:
        LimitedStreamWriter(StreamWriter& output, uint32_t length);
        LimitedStreamWriter(const LimitedStreamWriter& other);

    public:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

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
