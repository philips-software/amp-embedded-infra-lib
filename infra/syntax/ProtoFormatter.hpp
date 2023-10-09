#ifndef INFRA_PROTO_FORMATTER_HPP
#define INFRA_PROTO_FORMATTER_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedVector.hpp"

namespace infra
{
    class ProtoFormatter;

    uint32_t MaxVarIntSize(uint64_t value);

    class ProtoLengthDelimitedFormatter
    {
    public:
        ProtoLengthDelimitedFormatter(ProtoFormatter& formatter, uint32_t fieldNumber);
        ProtoLengthDelimitedFormatter(ProtoFormatter& formatter, uint32_t fieldNumber, std::size_t size);
        ProtoLengthDelimitedFormatter(const ProtoLengthDelimitedFormatter& other) = delete;
        ProtoLengthDelimitedFormatter(ProtoLengthDelimitedFormatter&& other) noexcept;
        ProtoLengthDelimitedFormatter& operator=(const ProtoLengthDelimitedFormatter& other) = delete;
        ~ProtoLengthDelimitedFormatter();

    private:
        ProtoFormatter& formatter;
        std::size_t marker;
    };

    class ProtoFormatter
    {
    public:
        explicit ProtoFormatter(infra::DataOutputStream& output);

        void PutVarInt(uint64_t value);
        void PutSignedVarInt(uint64_t value);
        void PutFixed32(uint32_t value);
        void PutFixed64(uint64_t value);
        void PutString(infra::BoundedConstString string);
        void PutBytes(infra::ConstByteRange bytes);

        void PutVarIntField(uint64_t value, uint32_t fieldNumber);
        void PutSignedVarIntField(uint64_t value, uint32_t fieldNumber);
        void PutFixed32Field(uint32_t value, uint32_t fieldNumber);
        void PutFixed64Field(uint64_t value, uint32_t fieldNumber);
        void PutLengthDelimitedField(infra::ConstByteRange range, uint32_t fieldNumber);
        void PutStringField(infra::BoundedConstString string, uint32_t fieldNumber);
        void PutBytesField(infra::ConstByteRange bytes, uint32_t fieldNumber);
        void PutLengthDelimitedSize(std::size_t size, uint32_t fieldNumber);
        ProtoLengthDelimitedFormatter LengthDelimitedFormatter(uint32_t fieldNumber);

    private:
        friend class ProtoLengthDelimitedFormatter;
        infra::DataOutputStream output;
    };
}

#endif
