#ifndef PROTOBUF_PROTO_MESSAGE_RECEIVER_HPP
#define PROTOBUF_PROTO_MESSAGE_RECEIVER_HPP

#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/Proto.hpp"

namespace services
{
    class ProtoMessageReceiverBase
    {
    public:
        explicit ProtoMessageReceiverBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>>& stack);

        void Feed(infra::ConstByteRange data);

    protected:
        template<class Message>
        void FeedForMessage(const infra::DataInputStream& stream, Message& message);

    private:
        template<class Message, std::size_t... I>
        bool DeserializeFields(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message, std::index_sequence<I...>);

        template<std::size_t I, class Message>
        bool DeserializeSingleField(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message);

        void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, bool& value) const;
        void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value) const;
        void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value) const;
        void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value) const;
        void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value) const;
        void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value) const;
        void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value) const;
        void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value) const;
        void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value) const;

        void DeserializeField(ProtoStringBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedString& value);
        void DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, std::string& value);
        void DeserializeField(ProtoBytesBase, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, infra::BoundedVector<uint8_t>& value);

        template<class Enum>
        void DeserializeField(ProtoEnum<Enum>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Enum& value) const;
        template<class Message>
        void DeserializeField(ProtoMessage<Message>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Message& value);
        template<class ProtoType, class Type>
        void DeserializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Type& value) const;
        template<class ProtoType, class Type>
        void DeserializeField(ProtoUnboundedRepeated<ProtoType>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Type& value) const;

        void ConsumeUnknownField(infra::ProtoParser::PartialField& field);

    private:
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> buffer;
        infra::BoundedVector<std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>>& stack;
    };

    template<class Message>
    class ProtoMessageReceiver
        : public ProtoMessageReceiverBase
    {
    public:
        ProtoMessageReceiver();

    public:
        Message message;

    private:
        infra::BoundedVector<std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>>::WithMaxSize<MessageDepth<services::ProtoMessage<Message>>::value + 1> stack{ { std::pair<uint32_t, infra::Function<void(const infra::DataInputStream& stream)>>{ std::numeric_limits<uint32_t>::max(), [this](const infra::DataInputStream& stream)
            {
                FeedForMessage(stream, message);
            } } } };
    };
}

//// Implementation ////

namespace services
{
    template<class Message>
    void ProtoMessageReceiverBase::FeedForMessage(const infra::DataInputStream& stream, Message& message)
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
    bool ProtoMessageReceiverBase::DeserializeFields(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message, std::index_sequence<I...>)
    {
        return (DeserializeSingleField<I>(field, parser, message) || ...);
    }

    template<std::size_t I, class Message>
    bool ProtoMessageReceiverBase::DeserializeSingleField(infra::ProtoParser::PartialField& field, infra::ProtoParser& parser, Message& message)
    {
        if (field.second == Message::template fieldNumber<I>)
        {
            DeserializeField(typename Message::template ProtoType<I>(), parser, field.first, message.Get(std::integral_constant<uint32_t, I>()));
            return true;
        }

        return false;
    }

    template<class Enum>
    void ProtoMessageReceiverBase::DeserializeField(ProtoEnum<Enum>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Enum& value) const
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<Enum>(field.Get<uint64_t>());
    }

    template<class Message>
    void ProtoMessageReceiverBase::DeserializeField(ProtoMessage<Message>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Message& value)
    {
        parser.ReportFormatResult(field.Is<infra::PartialProtoLengthDelimited>());
        if (field.Is<infra::PartialProtoLengthDelimited>())
        {
            auto messageSize = field.Get<infra::PartialProtoLengthDelimited>().length;
            stack.emplace_back(messageSize, [this, &value](const infra::DataInputStream& stream)
                {
                    FeedForMessage(stream, value);
                });
            infra::ReConstruct(value);
        }
    }

    template<class ProtoType, class Type>
    void ProtoMessageReceiverBase::DeserializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Type& value) const
    {
        parser.ReportFormatResult(!value.full());
        if (!value.full())
        {
            value.emplace_back();
            DeserializeField(ProtoType(), parser, field, value.back());
        }
    }

    template<class ProtoType, class Type>
    void ProtoMessageReceiverBase::DeserializeField(ProtoUnboundedRepeated<ProtoType>, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, Type& value) const
    {
        value.emplace_back();
        DeserializeField(ProtoType(), parser, field, value.back());
    }

    template<class Message>
    ProtoMessageReceiver<Message>::ProtoMessageReceiver()
        : ProtoMessageReceiverBase(stack)
    {}
}

#endif
