#include "protobuf/echo/ProtoMessageBuilder.hpp"

namespace services
{
    BufferingStreamReader::BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::ConstByteRange& data)
        : buffer(buffer)
        , data(data)
    {}

    void BufferingStreamReader::ConsumeCurrent()
    {
        std::size_t bufferDecrease = std::min(buffer.size(), index);
        buffer.erase(buffer.begin(), buffer.begin() + bufferDecrease);
        index -= bufferDecrease;

        data.pop_front(index);
        index = 0;
    }

    void BufferingStreamReader::StoreRemainder()
    {
        ConsumeCurrent();

        buffer.insert(buffer.end(), data.begin(), data.end());
        data.clear();
    }

    void BufferingStreamReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        if (index != buffer.size())
        {
            auto from = infra::Head(buffer.contiguous_range(buffer.begin() + index), range.size());
            infra::Copy(from, infra::Head(range, from.size()));
            range.pop_front(from.size());
            index += from.size();

            // Perhaps the deque just wrapped around, try once more
            from = infra::Head(buffer.contiguous_range(buffer.begin() + index), range.size());
            infra::Copy(from, infra::Head(range, from.size()));
            range.pop_front(from.size());
            index += from.size();
        }

        if (!range.empty())
        {
            auto dataIndex = index - buffer.size();
            auto from = infra::Head(infra::DiscardHead(data, dataIndex), range.size());
            infra::Copy(from, infra::Head(range, from.size()));
            range.pop_front(from.size());
            index += from.size();
        }

        errorPolicy.ReportResult(range.empty());
    }

    uint8_t BufferingStreamReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        auto range = PeekContiguousRange(0);

        errorPolicy.ReportResult(!range.empty());

        if (range.empty())
            return 0;
        else
            return range.front();
    }

    infra::ConstByteRange BufferingStreamReader::ExtractContiguousRange(std::size_t max)
    {
        if (index < buffer.size())
        {
            auto from = infra::Head(buffer.contiguous_range(buffer.begin() + index), max);
            index += from.size();
            return from;
        }

        auto dataIndex = index - buffer.size();
        auto from = infra::Head(infra::DiscardHead(data, dataIndex), max);
        index += from.size();
        return from;
    }

    infra::ConstByteRange BufferingStreamReader::PeekContiguousRange(std::size_t start)
    {
        if (index + start < buffer.size())
        {
            auto from = buffer.contiguous_range(buffer.begin() + index + start);
            return from;
        }

        auto dataIndex = index + start - buffer.size();
        auto from = infra::DiscardHead(data, dataIndex);
        return from;
    }

    bool BufferingStreamReader::Empty() const
    {
        return Available() == 0;
    }

    std::size_t BufferingStreamReader::Available() const
    {
        return buffer.size() + data.size() - index;
    }

    std::size_t BufferingStreamReader::ConstructSaveMarker() const
    {
        return index;
    }

    void BufferingStreamReader::Rewind(std::size_t marker)
    {
        index = marker;
    }

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
