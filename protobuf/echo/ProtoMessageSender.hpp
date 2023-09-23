#ifndef PROTOBUF_PROTO_MESSAGE_SENDER_HPP
#define PROTOBUF_PROTO_MESSAGE_SENDER_HPP

#include "infra/stream/CountingOutputStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/Proto.hpp"

namespace services
{
    class ProtoMessageSenderBase
    {
    public:
        explicit ProtoMessageSenderBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter), 3 * sizeof(uint8_t*)>>>& stack);

        void Fill(infra::DataOutputStream output);

    protected:
        template<class Message>
        bool FillForMessage(infra::DataOutputStream& stream, const Message& message, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter) const;

    private:
        template<class Message, std::size_t... I>
        bool SerializeFields(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter, std::index_sequence<I...>) const;

        template<std::size_t I, class Message>
        bool SerializeSingleField(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter) const;

        bool SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoInt32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoUInt64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoInt64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoFixed32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoSFixed32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoFixed64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber, const bool& retry) const;
        bool SerializeField(ProtoSFixed64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber, const bool& retry) const;

        bool SerializeField(ProtoStringBase, infra::ProtoFormatter& formatter, const infra::BoundedString& value, uint32_t fieldNumber, bool& retry) const;
        bool SerializeField(ProtoUnboundedString, infra::ProtoFormatter& formatter, const std::string& value, uint32_t fieldNumber, bool& retry) const;
        bool SerializeField(ProtoBytesBase, infra::ProtoFormatter& formatter, const infra::BoundedVector<uint8_t>& value, uint32_t fieldNumber, bool& retry) const;

        template<class Enum>
        bool SerializeField(ProtoEnum<Enum>, infra::ProtoFormatter& formatter, const Enum& value, uint32_t fieldNumber, bool& retry) const;
        template<class Message>
        bool SerializeField(ProtoMessage<Message>, infra::ProtoFormatter& formatter, const Message& value, uint32_t fieldNumber, bool& retry) const;
        template<class ProtoType, class Type>
        bool SerializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoFormatter& formatter, const infra::BoundedVector<Type>& value, uint32_t fieldNumber, bool& retry) const;
        template<class ProtoType, class Type>
        bool SerializeField(ProtoUnboundedRepeated<ProtoType>, infra::ProtoFormatter& formatter, const std::vector<Type>& value, uint32_t fieldNumber, bool& retry) const;

    private:
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> buffer;
        infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter), 3 * sizeof(uint8_t*)>>>& stack;
    };

    template<class Message>
    class ProtoMessageSender
        : public ProtoMessageSenderBase
    {
    public:
        explicit ProtoMessageSender(const Message& message);

    private:
        const Message& message;
        infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter), 3 * sizeof(uint8_t*)>>>::WithMaxSize<MessageDepth<services::ProtoMessage<Message>>::value + 1> stack{ { std::pair<uint32_t, infra::Function<bool(infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter), 3 * sizeof(uint8_t*)>>{ 0, [this](infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter)
            {
                return FillForMessage(stream, message, index, retry, finalWriter);
            } } } };
    };

    //// Implementation ////

    template<class Message>
    bool ProtoMessageSenderBase::FillForMessage(infra::DataOutputStream& stream, const Message& message, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter) const
    {
        infra::ProtoFormatter formatter{ stream };

        return SerializeFields(formatter, message, index, retry, finalWriter, std::make_index_sequence<Message::numberOfFields>{});
    }

    template<class Message, std::size_t... I>
    bool ProtoMessageSenderBase::SerializeFields(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter, std::index_sequence<I...>) const
    {
        return (SerializeSingleField<I>(formatter, message, index, retry, finalWriter) && ...);
    }

    template<std::size_t I, class Message>
    bool ProtoMessageSenderBase::SerializeSingleField(infra::ProtoFormatter& formatter, const Message& message, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter) const
    {
        if (finalWriter.Available() == 0)
            return false;

        if (index == I)
        {
            if (!SerializeField(typename Message::template ProtoType<I>(), formatter, message.Get(std::integral_constant<uint32_t, I>()), Message::template fieldNumber<I>, retry))
                return false;

            index = I + 1;
            return !retry;
        }

        return true;
    }

    template<class Enum>
    bool ProtoMessageSenderBase::SerializeField(ProtoEnum<Enum>, infra::ProtoFormatter& formatter, const Enum& value, uint32_t fieldNumber, bool& retry) const
    {
        services::SerializeField(ProtoEnum<Enum>(), formatter, value, fieldNumber);
        return true;
    }

    template<class Message>
    bool ProtoMessageSenderBase::SerializeField(ProtoMessage<Message>, infra::ProtoFormatter& formatter, const Message& value, uint32_t fieldNumber, bool& retry) const
    {
        infra::DataOutputStream::WithWriter<infra::CountingStreamWriter> countingStream;
        infra::ProtoFormatter countingFormatter{ countingStream };
        value.Serialize(countingFormatter);
        formatter.PutLengthDelimitedSize(countingStream.Writer().Processed(), fieldNumber);

        stack.emplace_back(0, [this, &value](infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter)
            {
                return FillForMessage(stream, value, index, retry, finalWriter);
            });

        retry = true;
        return true;
    }

    template<class ProtoType, class Type>
    bool ProtoMessageSenderBase::SerializeField(ProtoRepeatedBase<ProtoType>, infra::ProtoFormatter&, const infra::BoundedVector<Type>& value, uint32_t fieldNumber, bool& retry) const
    {
        stack.emplace_back(0, [this, &value, fieldNumber](infra::DataOutputStream& stream, uint32_t& index, const bool&, const infra::StreamWriter& finalWriter)
            {
                infra::ProtoFormatter formatter{ stream };

                for (; index != value.size(); ++index)
                {
                    if (finalWriter.Available() == 0)
                        return false;
                    services::SerializeField(ProtoType(), formatter, value[index], fieldNumber);
                }

                return true;
            });

        retry = true;
        return true;
    }

    template<class ProtoType, class Type>
    bool ProtoMessageSenderBase::SerializeField(ProtoUnboundedRepeated<ProtoType>, infra::ProtoFormatter&, const std::vector<Type>& value, uint32_t fieldNumber, bool& retry) const
    {
        stack.emplace_back(0, [this, &value, fieldNumber](infra::DataOutputStream& stream, uint32_t& index, const bool&, const infra::StreamWriter& finalWriter)
            {
                infra::ProtoFormatter formatter{ stream };

                for (; index != value.size(); ++index)
                {
                    if (finalWriter.Available() == 0)
                        return false;
                    services::SerializeField(ProtoType(), formatter, value[index], fieldNumber);
                }

                return true;
            });

        retry = true;
        return true;
    }

    template<class Message>
    ProtoMessageSender<Message>::ProtoMessageSender(const Message& message)
        : ProtoMessageSenderBase(stack)
        , message(message)
    {}
}

#endif
