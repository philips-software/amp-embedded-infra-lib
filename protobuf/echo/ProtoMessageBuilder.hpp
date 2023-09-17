#ifndef PROTOBUF_PROTO_MESSAGE_BUILDER_HPP
#define PROTOBUF_PROTO_MESSAGE_BUILDER_HPP

#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/Proto.hpp"

namespace services
{
    class BufferingStreamReader
        : public infra::StreamReaderWithRewinding
    {
    public:
        BufferingStreamReader(infra::BoundedDeque<uint8_t>& buffer, infra::ConstByteRange& data);

        void ConsumeCurrent();
        void StoreRemainder();

        // Implementation of StreamReaderWithRewinding
        void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
        infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
        infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
        bool Empty() const override;
        std::size_t Available() const override;
        std::size_t ConstructSaveMarker() const override;
        void Rewind(std::size_t marker) override;

    private:
        infra::BoundedDeque<uint8_t>& buffer;
        infra::ConstByteRange& data;
        std::size_t index = 0;
    };

    class ProtoMessageBuilderBase
    {
    public:
        ProtoMessageBuilderBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<void(infra::DataInputStream& stream)>>>& stack);

        void Feed(infra::ConstByteRange data);

    protected:
        template<class Message>
        void FeedForMessage(infra::DataInputStream& stream, Message& message);

    private:
        template<class Message, std::size_t... I>
        bool DeserializeFields(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message, std::index_sequence<I...>);

        template<std::size_t I, class Message>
        bool DeserializeSingleField(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message);

        void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, bool& value);
        void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value);
        void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value);
        void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value);
        void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value);
        void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value);
        void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value);
        void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value);
        void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value);

        void DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, std::string& value);
        void DeserializeField(ProtoBytesBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedVector<uint8_t>& value);

        template<class Message>
        void DeserializeField(ProtoMessage<Message>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Message& value);

        template<class ProtoType, class Type>
        void DeserializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Type& value);

        void ConsumeUnknownField(infra::ProtoParser::PartialField& field);

    private:
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> buffer;
        infra::BoundedVector<std::pair<uint32_t, infra::Function<void(infra::DataInputStream& stream)>>>& stack;
    };

    template<class Message>
    class ProtoMessageBuilder
        : public ProtoMessageBuilderBase
    {
    public:
        ProtoMessageBuilder();

    public:
        Message message;

    private:
        infra::BoundedVector<std::pair<uint32_t, infra::Function<void(infra::DataInputStream& stream)>>>::WithMaxSize<MessageDepth<services::ProtoMessage<Message>>::value + 1> stack{ { std::pair<uint32_t, infra::Function<void(infra::DataInputStream& stream)>>{ std::numeric_limits<uint32_t>::max(), [this](infra::DataInputStream& stream)
            {
                FeedForMessage(stream, message);
            } } } };
    };
}

//// Implementation ////

namespace services
{
    template<class Message>
    void ProtoMessageBuilderBase::FeedForMessage(infra::DataInputStream& stream, Message& message)
    {
        infra::ProtoParser parser{ stream };

        while (!stream.Empty())
        {
            auto marker = static_cast<infra::StreamReaderWithRewinding&>(stream.Reader()).ConstructSaveMarker();
            auto field = parser.GetPartialField();

            if (stream.Failed())
            {
                static_cast<infra::StreamReaderWithRewinding&>(stream.Reader()).Rewind(marker);
                break;
            }

            auto stackSize = stack.size();
            if (!DeserializeFields(field, parser, message, std::make_index_sequence<Message::numberOfFields>{}))
                ConsumeUnknownField(field);

            if (stackSize != stack.size())
            {
                stream.Failed();
                return;
            }
        }
    }

    template<class Message, std::size_t... I>
    bool ProtoMessageBuilderBase::DeserializeFields(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message, std::index_sequence<I...>)
    {
        return (DeserializeSingleField<I>(field, parser, message) || ...);
    }

    template<std::size_t I, class Message>
    bool ProtoMessageBuilderBase::DeserializeSingleField(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message)
    {
        if (field.second == Message::template fieldNumber<I>)
        {
            DeserializeField(typename Message::template ProtoType<I>(), parser, field.first, message.Get(std::integral_constant<uint32_t, I>()));
            return true;
        }

        return false;
    }

    template<class Message>
    void ProtoMessageBuilderBase::DeserializeField(ProtoMessage<Message>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Message& value)
    {
        parser.ReportFormatResult(field.Is<infra::PartialProtoLengthDelimited>());
        if (field.Is<infra::PartialProtoLengthDelimited>())
        {
            auto messageSize = field.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(messageSize, [this, &value](infra::DataInputStream& stream)
                {
                    FeedForMessage(stream, value);
                });
            infra::ReConstruct(value);
        }
    }

    template<class ProtoType, class Type>
    void ProtoMessageBuilderBase::DeserializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Type& value)
    {
        parser.ReportFormatResult(!value.full());
        if (!value.full())
        {
            value.emplace_back();
            DeserializeField(ProtoType(), parser, field, value.back());
        }
    }

    template<class Message>
    ProtoMessageBuilder<Message>::ProtoMessageBuilder()
        : ProtoMessageBuilderBase(stack)
    {}
}

#endif
