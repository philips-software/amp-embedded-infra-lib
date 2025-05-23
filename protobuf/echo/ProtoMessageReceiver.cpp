#include "protobuf/echo/ProtoMessageReceiver.hpp"
#include "infra/stream/BufferingStreamReader.hpp"

namespace services
{
    ProtoMessageReceiverBase::ProtoMessageReceiverBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>>& stack)
        : stack(stack)
    {}

    void ProtoMessageReceiverBase::Feed(infra::StreamReaderWithRewinding& data)
    {
        infra::BufferingStreamReader reader{ buffer, data };

        while (true)
        {
            infra::LimitedStreamReaderWithRewinding limitedReader(reader, stack.back().first);
            infra::DataInputStream::WithErrorPolicy stream{ limitedReader, infra::softFail };

            auto available = limitedReader.Available();

            const auto& current = stack.back();
            current.second(stream);

            ConsumeStack(current, available - limitedReader.Available());

            if (stream.Failed() || reader.Empty())
            {
                failed = stream.Failed();
                break;
            }
        }
    }

    bool ProtoMessageReceiverBase::Failed() const
    {
        return failed;
    }

    void ProtoMessageReceiverBase::ConsumeStack(const std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>& current, std::size_t amount)
    {
        if (&current != &stack.front())
        {
            for (auto& i : stack)
            {
                i.first -= amount;

                if (&i == &current)
                    break;
            }

            while (stack.size() > 1 && stack.back().first == 0)
                stack.pop_back();
        }
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, bool& value) const
    {
        services::DeserializeField(ProtoBool(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value) const
    {
        services::DeserializeField(ProtoUInt32(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value) const
    {
        services::DeserializeField(ProtoInt32(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value) const
    {
        services::DeserializeField(ProtoUInt64(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value) const
    {
        services::DeserializeField(ProtoInt64(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value) const
    {
        services::DeserializeField(ProtoFixed32(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value) const
    {
        services::DeserializeField(ProtoFixed64(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value) const
    {
        services::DeserializeField(ProtoSFixed32(), parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value) const
    {
        services::DeserializeField(ProtoSFixed64(), parser, field, value);
    }

    namespace
    {
        template<class C>
        void DeserializeContainer(infra::BoundedVector<std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>>& stack, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, C& value)
        {
            parser.ReportFormatResult(field.Is<infra::PartialProtoLengthDelimited>());
            if (field.Is<infra::PartialProtoLengthDelimited>())
            {
                auto bytesSize = field.Get<infra::PartialProtoLengthDelimited>().length;
                stack.emplace_back(bytesSize, [&value](const infra::DataInputStream& stream)
                    {
                        while (!stream.Empty())
                        {
                            auto range = infra::ReinterpretCastMemoryRange<const typename C::value_type>(stream.ContiguousRange());
                            stream.ErrorPolicy().ReportResult(range.size() <= value.max_size() - value.size());
                            range.shrink_from_back_to(value.max_size() - value.size());
                            value.insert(value.end(), range.begin(), range.end());
                        }
                    });
                value.clear();
            }
        }
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoStringBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedString& value)
    {
        DeserializeContainer(stack, parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, std::string& value)
    {
        DeserializeContainer(stack, parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoBytesBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedVector<uint8_t>& value)
    {
        DeserializeContainer(stack, parser, field, value);
    }

    void ProtoMessageReceiverBase::DeserializeField(ProtoUnboundedBytes, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, std::vector<uint8_t>& value)
    {
        DeserializeContainer(stack, parser, field, value);
    }

    void ProtoMessageReceiverBase::ConsumeUnknownField(infra::ProtoParser::PartialField& field)
    {
        if (field.first.Is<infra::PartialProtoLengthDelimited>())
        {
            auto size = field.first.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(size, [](const infra::DataInputStream& stream)
                {
                    while (!stream.Empty())
                        stream.ContiguousRange();
                });
        }
    }
}
