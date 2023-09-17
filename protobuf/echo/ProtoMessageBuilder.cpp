#include "protobuf/echo/ProtoMessageBuilder.hpp"

namespace services
{
    ProtoMessageBuilderBase::ProtoMessageBuilderBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<void(infra::DataInputStream& stream)>>>& stack)
        : stack(stack)
    {}

    void ProtoMessageBuilderBase::Feed(infra::ConstByteRange data)
    {
        BufferingStreamReader reader{ buffer, data };

        while (!data.empty())
        {
            infra::LimitedStreamReaderWithRewinding limitedReader(reader, stack.back().first);
            infra::DataInputStream::WithErrorPolicy stream{ limitedReader, infra::softFail };

            auto available = limitedReader.Available();

            auto& current = stack.back();
            current.second(stream);

            if (stream.Failed() || reader.Empty())
            {
                reader.StoreRemainder();
                break;
            }

            if (&current != &stack.front())
            {
                current.first -= available - limitedReader.Available();

                if (current.first == 0)
                    stack.pop_back();
            }
        }
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, bool& value)
    {
        services::DeserializeField(ProtoBool(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value)
    {
        services::DeserializeField(ProtoUInt32(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value)
    {
        services::DeserializeField(ProtoInt32(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value)
    {
        services::DeserializeField(ProtoUInt64(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value)
    {
        services::DeserializeField(ProtoInt64(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value)
    {
        services::DeserializeField(ProtoFixed32(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value)
    {
        services::DeserializeField(ProtoFixed64(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value)
    {
        services::DeserializeField(ProtoSFixed32(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value)
    {
        services::DeserializeField(ProtoSFixed64(), parser, field, value);
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoStringBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedString& value)
    {
        parser.ReportFormatResult(field.Is<infra::PartialProtoLengthDelimited>());
        if (field.Is<infra::PartialProtoLengthDelimited>())
        {
            auto stringSize = field.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(stringSize, [this, &value](infra::DataInputStream& stream)
                {
                    while (!stream.Empty())
                        value.append(infra::ByteRangeAsString(stream.ContiguousRange()));
                });
            value.clear();
        }
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, std::string& value)
    {
        parser.ReportFormatResult(field.Is<infra::PartialProtoLengthDelimited>());
        if (field.Is<infra::PartialProtoLengthDelimited>())
        {
            auto stringSize = field.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(stringSize, [this, &value](infra::DataInputStream& stream)
                {
                    while (!stream.Empty())
                        value.append(infra::ByteRangeAsStdString(stream.ContiguousRange()));
                });
            value.clear();
        }
    }

    void ProtoMessageBuilderBase::DeserializeField(ProtoBytesBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedVector<uint8_t>& value)
    {
        parser.ReportFormatResult(field.Is<infra::PartialProtoLengthDelimited>());
        if (field.Is<infra::PartialProtoLengthDelimited>())
        {
            auto bytesSize = field.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(bytesSize, [this, &value](infra::DataInputStream& stream)
                {
                    while (!stream.Empty())
                    {
                        auto range = stream.ContiguousRange();
                        value.insert(value.end(), range.begin(), range.end());
                    }
                });
            value.clear();
        }
    }

    void ProtoMessageBuilderBase::ConsumeUnknownField(infra::ProtoParser::PartialField& field)
    {
        if (field.first.Is<infra::PartialProtoLengthDelimited>())
        {
            auto size = field.first.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(size, [this](infra::DataInputStream& stream)
                {
                    while (!stream.Empty())
                        stream.ContiguousRange();
                });
        }
    }
}
