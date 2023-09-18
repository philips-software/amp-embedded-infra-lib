#ifndef PROTOBUF_PROTO_MESSAGE_SENDER_HPP
#define PROTOBUF_PROTO_MESSAGE_SENDER_HPP

#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/Proto.hpp"

namespace services
{
    class ProtoMessageSenderBase
    {
    public:
        explicit ProtoMessageSenderBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(const infra::DataOutputStream& stream, uint32_t& index, bool& retry), 3 * sizeof(uint8_t*)>>>& stack);

        void Fill(infra::DataOutputStream output);

    protected:
        template<class Message>
        bool FillForMessage(const infra::DataOutputStream& stream, const Message& message, uint32_t& index, bool& retry);

    private:
        template<class Message, std::size_t... I>
        bool SerializeFields(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry, std::index_sequence<I...>);

        template<std::size_t I, class Message>
        bool SerializeSingleField(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry);

        bool SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber, bool& retry);
        bool SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber, bool& retry);

        template<class ProtoType, class Type>
        bool SerializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoFormatter& formatter, Type& value, uint32_t fieldNumber, bool& retry) const;

    private:
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> buffer;
        infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(const infra::DataOutputStream& stream, uint32_t& index, bool& retry), 3 * sizeof(uint8_t*)>>>& stack;
    };

    template<class Message>
    class ProtoMessageSender
        : public ProtoMessageSenderBase
    {
    public:
        ProtoMessageSender(const Message& message);

    public:
        const Message& message;

    private:
        infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(const infra::DataOutputStream& stream, uint32_t& index, bool& retry), 3 * sizeof(uint8_t*)>>>::WithMaxSize<MessageDepth<services::ProtoMessage<Message>>::value + 1> stack{ { std::pair<uint32_t, infra::Function<bool(const infra::DataOutputStream& stream, uint32_t& index, bool& retry), 3 * sizeof(uint8_t*)>>{ 0, [this](const infra::DataOutputStream& stream, uint32_t& index, bool& retry) { return FillForMessage(stream, message, index, retry); }}} };
    };

    //// Implementation ////

    template<class Message>
    bool ProtoMessageSenderBase::FillForMessage(const infra::DataOutputStream& stream, const Message& message, uint32_t& index, bool& retry)
    {
        infra::DataOutputStream streamCopy{ stream };
        infra::ProtoFormatter formatter{ streamCopy };

        //while (!stream.Empty())
        //{
        //    auto marker = static_cast<infra::StreamReaderWithRewinding&>(stream.Reader()).ConstructSaveMarker();
        //    auto field = parser.GetPartialField();

        //    if (stream.Failed())
        //    {
        //        static_cast<infra::StreamReaderWithRewinding&>(stream.Reader()).Rewind(marker);
        //        break;
        //    }

        //    auto stackSize = stack.size();
        return SerializeFields(formatter, message, index, retry, std::make_index_sequence<Message::numberOfFields>{});

        //    if (stackSize != stack.size())
        //    {
        //        stream.Failed();
        //        return;
        //    }
        //}
    }

    template<class Message, std::size_t... I>
    bool ProtoMessageSenderBase::SerializeFields(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry, std::index_sequence<I...>)
    {
        return (SerializeSingleField<I>(formatter, message, index, retry) && ...);
    }

    template<std::size_t I, class Message>
    bool ProtoMessageSenderBase::SerializeSingleField(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry)
    {
        if (!SerializeField(typename Message::template ProtoType<I>(), formatter, message.Get(std::integral_constant<uint32_t, I>()), Message::fieldNumber<I>, retry))
            return false;

        index = I + 1;
        return true;
    }

    template<class ProtoType, class Type>
    bool ProtoMessageSenderBase::SerializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoFormatter& formatter, Type& value, uint32_t fieldNumber, bool& retry) const
    {
        stack.emplace_back(0, [this, &value, fieldNumber](const infra::DataOutputStream& stream, uint32_t& index, bool& retry)
            {
                infra::DataOutputStream streamCopy{ stream };
                infra::ProtoFormatter formatter{ streamCopy };

                for (; index != value.size(); ++index)
                    services::SerializeField(ProtoType(), formatter, value[index], fieldNumber);

                return true;
            });

        retry = true;
        return false;
    }

    template<class Message>
    ProtoMessageSender<Message>::ProtoMessageSender(const Message& message)
        : ProtoMessageSenderBase(stack)
        , message(message)
    {}
}

#endif
