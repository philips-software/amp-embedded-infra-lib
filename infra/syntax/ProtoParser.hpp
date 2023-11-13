#ifndef INFRA_PROTO_PARSER_HPP
#define INFRA_PROTO_PARSER_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Variant.hpp"
#include <utility>

namespace infra
{
    class ProtoParser;

    class ProtoLengthDelimited
    {
    public:
        ProtoLengthDelimited(infra::DataInputStream inputStream, infra::StreamErrorPolicy& formatErrorPolicy, uint32_t length);
        ProtoLengthDelimited(const ProtoLengthDelimited& other);
        ~ProtoLengthDelimited() = default;

        uint32_t Available() const;
        void SkipEverything();
        ProtoParser Parser();
        void GetString(infra::BoundedString& string);
        void GetStringReference(infra::BoundedConstString& string);
        std::string GetStdString();
        void GetBytes(infra::BoundedVector<uint8_t>& bytes);
        void GetBytesReference(infra::ConstByteRange& bytes);
        std::vector<uint8_t> GetUnboundedBytes();

    private:
        infra::LimitedStreamReader limitedReader;
        infra::DataInputStream input;
        infra::StreamErrorPolicy& formatErrorPolicy;
    };

    struct PartialProtoLengthDelimited
    {
        uint32_t length;
    };

    class ProtoParser
    {
    public:
        using FieldVariant = infra::Variant<uint32_t, uint64_t, ProtoLengthDelimited>;
        using Field = std::pair<FieldVariant, uint32_t>;
        using PartialFieldVariant = infra::Variant<uint32_t, uint64_t, PartialProtoLengthDelimited>;
        using PartialField = std::pair<PartialFieldVariant, uint32_t>;

        explicit ProtoParser(infra::DataInputStream inputStream);
        ProtoParser(infra::DataInputStream inputStream, infra::StreamErrorPolicy& formatErrorPolicy);

        bool Empty() const;
        uint64_t GetVarInt();
        uint32_t GetFixed32();
        uint64_t GetFixed64();

        Field GetField();
        PartialField GetPartialField();

        void ReportFormatResult(bool ok);
        bool FormatFailed() const;

    private:
        infra::LimitedStreamReader limitedReader;
        infra::DataInputStream input;
        infra::StreamErrorPolicy& formatErrorPolicy;
    };
}

#endif
